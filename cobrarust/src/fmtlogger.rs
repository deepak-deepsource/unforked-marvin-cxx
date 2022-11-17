use log::{Level, LevelFilter, Log, Metadata, Record};

/// Implements [`Log`] and a set of simple builder methods for configuration.
struct Logger {
    /// Global logging level when using this type
    level: LevelFilter,
}

impl Default for Logger {
    fn default() -> Self {
        Logger {
            level: LevelFilter::Trace,
        }
    }
}

impl Log for Logger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        &metadata.level().to_level_filter() <= &self.level
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let target = if record.target().is_empty() {
                record.module_path().unwrap_or_default()
            } else {
                record.target()
            };

            const BLUE: &str = "\x1B[34m";
            const RED: &str = "\x1B[31m";
            const YELLOW: &str = "\x1B[33m";
            const WHITE: &str = "\x1B[37m";
            const RESET: &str = "\x1B[0m";

            let color = match record.level() {
                Level::Error => RED,
                Level::Warn => YELLOW,
                Level::Info => WHITE,
                Level::Debug => BLUE,
                Level::Trace => "",
            };
 
            eprintln!("{}[{}] {}{}", color, target, record.args(), RESET);
        }
    }

    fn flush(&self) {}
}

pub fn default() {
    const LOGGER: Logger = Logger {
        level: LevelFilter::Trace,
    };    
    log::set_max_level(LOGGER.level);
    if let Err(err) = log::set_logger(&LOGGER) {
        eprintln!("attaching logger failed! shouldn't be possible: {:?}", err);
    }
}
