#!/bin/bash
#############################################################################
# This script is meant to be a single point entry for building, running or
# testing marvin-cxx
#
# Add support for:
#   * cobra run
#   * selectively run cobra and cxxengine
#   * run only a single issue code
#############################################################################

# Ref: https://gist.github.com/robin-a-meade/58d60124b88b60816e8349d1e3938615
# NOTE: This might be "too" strict in the future
set -euo pipefail

# make
if [ -z "${LLVM_INSTALL_DIR}" ]; then
  echo "$LLVM_INSTALL_DIR not set! Exiting."
  exit
fi

# common vars
MARVINCXX_EXEC_PATH=${PWD}/build/src/engine/cxxengine
TOOLBOX_PATH=/tmp/toolbox
INIT_LOG=${TOOLBOX_PATH}/init_go.log
REPO_PATH=/tmp/code
CHECK_TEST_FILES_PATH=${PWD}/test/checks
CORES=1

function phelp() {
  printf "usage: ./build.sh [--cc|--run <username/reponame>|--test|--test-checks|--clean-first]\n"
  printf "\t--cc                        build compile_commands.json in current dir\n"
  printf "\t--run <username/reponame>   build and run the LTA and cargo (git repo name should be username/reponame)\n"
  printf "\t--test                      build and run all tests\n"
  printf "\t--test-checks               build and run test for checks\n"
  printf "\t--clean-first               delete build dir and re-build\n"
}

function cmake_configure() {
  CC="${LLVM_INSTALL_DIR}/bin/clang" CXX="${LLVM_INSTALL_DIR}/bin/clang++" \
    cmake \
      -DLLVM_DIR="${LLVM_INSTALL_DIR}/lib/cmake/llvm" \
      -DLLVM_INSTALL_DIR="${LLVM_INSTALL_DIR}" \
      -S . \
      -B build
}

function set_cores() {
  OSNAME="$(uname)"
  if [ "${OSNAME}" == "Linux" ]; then
    # TODO: Check for Debain
    CORES=$(nproc --all)
  elif [ "${OSNAME}" == "Darwin" ]; then
    CORES=$(sysctl -n hw.logicalcpu)
  else
    echo "Unsupported OS '${OSNAME}'"
  fi
}

function cmake_build() {
  CC="${LLVM_INSTALL_DIR}/bin/clang" CXX="${LLVM_INSTALL_DIR}/bin/clang++" \
    cmake \
       --build build \
       --parallel "${CORES}"
}

function cmake_clean_build() {
  CC="${LLVM_INSTALL_DIR}/bin/clang" CXX="${LLVM_INSTALL_DIR}/bin/clang++" \
    cmake \
       --build build \
       --parallel "${CORES}" \
       --clean-first
}

function run_all_test() {
  echo "Test is not supported! Feel free to add the support!"
}

function get_repo {
  if [ -d "${REPO_PATH}" ]; then
    rm -rf "${REPO_PATH}"
  fi

  mkdir "${REPO_PATH}"

  git clone "git@github.com:$REMOTE_PATH" $REPO_PATH
  if [ "$?" != "0" ]; then
    echo "Failed to clone $REMOTE_PATH! Exiting"
    return 1
  fi
}

function write_analysis_config {
  if [ ! -d "${TOOLBOX_PATH}" ]; then
    echo "Missing $TOOLBOX_PATH! Can't write analysis config"
    return 1
  fi

  if [ ! -d "${CODE_PATH}" ]; then
    echo "Missing $CODE_PATH! Can't write analysis config"
    return 2
  fi

  # Create analysis config file
  # skipcq: SH-2086
  FILES="$(find ${CODE_PATH} -type f \( -name \*.cpp -o -name \*.cc -o -name \*.c -o -name \*.h -o -name \*.hpp \))"

  SEPERATED_FILES=""
  for f in ${FILES}; do
    SEPERATED_FILES+="\"${f}\", "
  done
  SEPERATED_FILES=${SEPERATED_FILES%", "}

  echo "{
    \"files\": [
  ${SEPERATED_FILES}
    ],
    \"analyzer_meta\": {
      \"name\": \"cxx\",
      \"enabled\": true,
      \"meta\": {}
    },
    \"version\": 0,
    \"transformers\": [
    ]
  }" > "${TOOLBOX_PATH}/analysis_config.json"
  # Done creating analysis config
}

function setup_toolbox {
  if [ -d "${TOOLBOX_PATH}" ]; then
    rm -rf $TOOLBOX_PATH
  fi

  mkdir "${TOOLBOX_PATH}"

  echo "--> Using cxxengine from ${MARVINCXX_EXEC_PATH}"
  # Copy the cxxengine for init.go to pick it up
  cp "${MARVINCXX_EXEC_PATH}" "${TOOLBOX_PATH}"
}

function main() {
  set_cores
  if [ "$#" == "0" ]; then
    cmake_configure
    cmake_build
    return 0
  fi

  case $1 in
    "--cc")
      cmake_configure
      cp build/compile_commands.json .
      echo "Created ${PWD}/compile_commands.json"
      ;;

    "--clean-first")
      cmake_configure
      cmake_clean_build
      ;;

    "--analyse-checker-test-files")
      # note path only run the cxxengine from CHECK_TEST_FILES_PATH
      # for validation of generate results use the tool "scatr"
      # this is only meant for scatr, to test the full system using --test
      setup_toolbox
      CODE_PATH=${CHECK_TEST_FILES_PATH} write_analysis_config
      TOOLBOX_PATH=${TOOLBOX_PATH} CODE_PATH=${CHECK_TEST_FILES_PATH} go run "${PWD}" &> "${INIT_LOG}"
      ;;

    "--test")
      run_all_test
      ;;

    "--test-checks")
      scatr run
      ;;

    "--run")
      # set-up repo path
      if [ "$#" == "2" ]; then
        REMOTE_PATH="$2"
      else
        GITUSERNAME="$(git config user.name)"
        REMOTE_PATH="${GITUSERNAME}/rotten-cxx"
        echo "--> Using git user ${GITUSERNAME} to fetch ${REMOTE_PATH}"
      fi

      # run cmake config
      cmake_configure
      # run cmake build
      cmake_build
      # create toolbox dir
      setup_toolbox
      # fetch src under investigation
      CODE_PATH="${REPO_PATH}" REMOTE_PATH="${REMOTE_PATH}" get_repo
      # build analysis_config using src under investigation
      CODE_PATH="${REPO_PATH}" write_analysis_config
      # run analysis
      TOOLBOX_PATH="${TOOLBOX_PATH}" CODE_PATH="${REPO_PATH}" \
        go run "${PWD}"
      echo
      echo "See ${TOOLBOX_PATH}/analysis_results.json"
      ;;

    *)
      echo "Unkwown option: $1"
      phelp
      ;;

  esac
}

main "$@"
