FROM rust:slim-bullseye AS rust_builder
USER root
# included for openssl-sys lib used by sentry
# can use rustls for completely static builds without any depends
# should work without this as sentry is no longer in build
# revert if adding sentry back
# RUN apt update && \
#     apt install -y openssl libssl-dev pkg-config
WORKDIR /app
ENV RUSTFLAGS="-C target-feature=-crt-static" 

ADD ./cobrarust/ .
RUN cargo build --release

#
# Phase: Build libtooling based analyser 
#
FROM debian:bullseye as debian_llvm
ENV DEBIAN_FRONTEND=noninteractive

# update the system and install any dependencies
RUN apt update \
    && apt install -y git bash cmake curl build-essential byacc openssh-client grep lsb-release wget software-properties-common gnupg # skipcq: DOK-DL3018

# Get LLVM
ARG LLVM_VER=15
RUN wget https://apt.llvm.org/llvm.sh

RUN chmod +x ./llvm.sh
RUN ./llvm.sh ${LLVM_VER} all

# Add environment variables for build
ENV PATH=$PATH:/usr/lib/llvm-${LLVM_VER}/bin
ENV LLVM_INSTALL_DIR /usr/lib/llvm-${LLVM_VER}

FROM debian_llvm as lta_builder

# Init working dir to build
RUN mkdir -p /lta_src/src /is_cpp
WORKDIR /lta_src

# Copy artifacts of libtooling based analyser
COPY ./src/ /lta_src/src/
COPY build.sh CMakeLists.txt /lta_src/

# Build the libtooling based analyser
RUN ./build.sh

# Build is_cpp tool
WORKDIR /is_cpp
COPY ./is_cpp/ /is_cpp/
RUN ./build.sh

#
# End of phase
#

### Build the entry point
# Stage 2
FROM golang:1.19.2-bullseye AS go_builder

RUN mkdir /app /code

COPY . /code
WORKDIR /code

# Build the go source
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o /app/marvin-cxx .

### Final stage
FROM debian_llvm

USER root

RUN rm -rf /code/ /toolbox/
RUN mkdir -p /home/runner /code /toolbox /toolbox/run

# Necessary dependencies

RUN cd /toolbox/ \
    && git clone --depth=1 https://github.com/nimble-code/Cobra.git ./cobra \
    && cd ./cobra/src \
    && make \
    && cd .. \
    && cp ./src/cobra . \
    && chmod +x ./cobra

# Copy the builds
## Note: Copy should be before setting permissions so that permissions get inherited on `chown -R` call
COPY --from=rust_builder /app/target/release/cobrarust /toolbox/
COPY --from=lta_builder /lta_src/build/src/engine/cxxengine /toolbox
COPY --from=lta_builder /is_cpp/is_cpp /toolbox
COPY --from=go_builder /app/marvin-cxx /app/
COPY .deepsource/analyzer/silencers.json /toolbox/

RUN /toolbox/cobra/cobra -configure /toolbox/cobra

# Set permissions and create user to run analyzers
RUN useradd -s /bin/bash -u 1000 runner \
    && chown -R runner:runner /home/runner /toolbox /code \
    && chmod -R o-rwx /code /toolbox

USER runner
