mod message_mapping;
mod result;
mod txt_parser;
mod fmtlogger;

use std::{
    collections::HashMap,
    fs::create_dir,
    io::Write,
    path::{Path, PathBuf},
    process::{self, Command, Output},
    error::Error,
};

fn run_cobra_at<S1, S2, S3, P>(
    path_to_cobra: S1,
    check: S2,
    run_dir: S3,
    files: &[P],
) -> Result<Output, Box<dyn Error>>
where
    S1: AsRef<str>,
    S2: AsRef<str>,
    S3: AsRef<str>,
    P: AsRef<Path>,
{
    let command = format!(
        "cd {} && {} {} {} {} {}",
        run_dir.as_ref(),
        path_to_cobra.as_ref(),
        "-terse",
        "-f",
        check.as_ref(),
        files.into_iter().fold(String::new(), |mut acc, x| {
            acc.push_str(&format!("{}", x.as_ref().display()));
            acc.push(' ');
            acc
        })
    );
    let ret = Command::new("sh").arg("-c").arg(command).output()?;
    log::info!("\ncommand output:: {ret:#?}\n");
    Ok(ret)
}

fn run_checks<P: AsRef<Path>>(run_dir: &str, checks: Vec<&str>, files: Vec<P>) -> Vec<Output> {
    let path_to_cobra = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "/toolbox/cobra/cobra".to_string());
    checks
        .iter()
        .filter_map(|check| {
            run_cobra_at(&path_to_cobra, check, run_dir, &files)
                .map_err(|x| {
                    log::info!("running cobra failed: {x}");
                    x
                })
                .ok()
        })
        .collect()
}

fn get_files_from_folder<P: AsRef<Path>>(
    folder: P,
    includes: &[&str],
    excludes: &[&str],
) -> Vec<PathBuf> {
    // extensions [".h", ".c", ".cpp", ".hpp", ".m"]
    let root = walkdir::WalkDir::new(folder);
    root.into_iter()
        .flatten() // remove err files
        // IMP: Will break on symblinks the assumption is there are no symblinks
        .filter(|entry| entry.file_type().is_file())
        // IMP: Will only work for files on the fs, ala, can have absolute paths
        // will ignore all the other files in the dir.
        .filter_map(|entry| std::fs::canonicalize(entry.path()).ok())
        .filter(|file| {
            if let Some(filepath) = file.to_str() {
                if includes.iter().all(|inc| filepath.contains(inc))
                    && !excludes.iter().all(|inc| filepath.contains(inc))
                {
                    true
                } else {
                    false
                }
            } else {
                false
            }
        })
        .collect()
}

fn main() {
    // setup logging
    fmtlogger::default();

    // all errors are propagated to sentry with backtrace
    if let Err(err) = _main() {
        log::error!("error raised: {err}");
        // early exit with status 1
        process::exit(1);
    }
}

fn configure_cobra<S: AsRef<str>>(cobra_folder: S) {
    let path_to_cobra = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "/toolbox/cobra/cobra".to_string());
    let command = format!("{} -configure {}", path_to_cobra, cobra_folder.as_ref());
    let ret = Command::new("sh").arg("-c").arg(command).output();
    log::info!("configure out:: {:?}", ret);
}

fn _main() -> Result<(), Box<dyn Error>> {
    // create dir for running and fetching results from cobra
    let run_dir = std::fs::canonicalize(PathBuf::from("./run")).unwrap_or_else(|_| {
        log::error!(
            "run folder not found in toolbox folder, attempting to create it now, {:?}",
            create_dir("/toolbox/run")
        );
        PathBuf::from("/toolbox/run")
    });
    // clean dir before use
    let _ = std::fs::remove_dir_all(&run_dir).and_then(|_| std::fs::create_dir(&run_dir));
    log::info!("rundir is {:?}", run_dir.to_str());

    configure_cobra(
        run_dir
            .parent()
            .unwrap_or(&PathBuf::from("/toolbox"))
            .join("cobra")
            .to_str()
            .unwrap_or("/toolbox/cobra"),
    );

    // use path for providing autosar check paths
    // let path_to_cobra = std::env::args()
    //     .nth(1)
    //     .unwrap_or_else(|| "/toolbox/cobra/cobra".to_string());
    run_checks(
        &format!("{}", run_dir.display()),
        vec!["cwe", "basic", "suspicious"],
        std::env::args()
            .nth(2)
            .unwrap_or_default()
            .split(' ')
            .collect(),
    );

    let files = get_files_from_folder(&run_dir, &[], &["detail"]);

    let mut lint_map = HashMap::new();
    for file in files {
        log::info!("file cobra ran on:: {}", file.display());
        lint_map.extend(txt_parser::parse(std::fs::read_to_string(file)?)?);
    }

    let issues = lint_map.keys().filter_map(|x| {
        Some((
            message_mapping::MESSAGE_MAPS
                .iter()
                .find_map(|map| map.get(x))?,
            x.clone(),
            lint_map[x].clone(),
        ))
    });

    let mut issue_occurrences = vec![];

    for (issue_code, message, occurrences) in issues {
        for txt_parser::Occurrence { file, begin, end } in occurrences {
            issue_occurrences.push(result::Issue {
                issue_text: message.clone(),
                issue_code: issue_code.to_string(),
                location: result::Location {
                    path: file,
                    position: result::Position { begin, end },
                },
            });
        }
    }
    log::debug!("\nissues raised are: {:#?}\n", issue_occurrences);

    // try and clean the run dir after use
    let _ = std::fs::remove_dir_all(&run_dir).and_then(|_| std::fs::create_dir(&run_dir));

    let src = serde_json::to_string(&issue_occurrences).unwrap_or_else(|err| {
        log::error!("{err}");
        "{}".to_string()
    });

    print!("{}", src);
    // let mut file = std::fs::File::create(std::path::PathBuf::from(
    //     std::env::args().nth(3).unwrap_or_else(|| "/toolbox".to_string()),
    // ).join("/cobra_result.json"))?;
    // write!(file, "{}", src)?;

    Ok(())
}
