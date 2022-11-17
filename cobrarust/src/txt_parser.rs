#![allow(unused)]
use std::{collections::HashMap, error::Error};

use crate::result::Mark;

///! Format:
///! ```txt
///! === MESSAGE ===
///! OCCURRENCE_COUNT: FILE_NAME:RANGE_IN_LINES (LINE_COUNT lines)
///!   LINE_NO     CODE_SNIPPET
///! ```

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Occurrence {
    pub file: String,
    pub begin: Mark,
    pub end: Mark,
}

fn get_range<S: AsRef<str>>(rn: S) -> Option<(Mark, Mark)> {
    // split will always return atleast 1 output will never panic
    let r = rn.as_ref().trim().split(' ').nth(0).unwrap();
    let rs = r.split("..").collect::<Vec<_>>();
    if rs.len() == 2 {
        let ss = u32::from_str_radix(rs[0], 10).ok()?;
        let vs = u32::from_str_radix(rs[1], 10).ok()?;
        Some((
            Mark {
                line: ss,
                column: 0,
            },
            Mark {
                line: vs,
                column: 0,
            },
        ))
    } else {
        None
    }
}

fn get_message<S: AsRef<str>>(msg: S) -> Option<String> {
    let msg = msg.as_ref();
    if msg.starts_with("===") && msg.ends_with("===") {
        Some(
            msg.trim_start_matches("===")
                .trim_end_matches("===")
                .trim()
                .to_owned(),
        )
    } else {
        None
    }
}

/// parse the string source received
/// this is a fairly ok parsing strategy with the goal of minimizing extra code.
pub(crate) fn parse<S: AsRef<str>>(src: S) -> Result<HashMap<String, Vec<Occurrence>>, Box<dyn Error>> {
    let mut map = HashMap::<String, Vec<Occurrence>>::new();
    // TODO: enable suppresion interface
    // let suppresion_map = suppression_map();
    let mut current_message = String::new();
    let lines = src.as_ref().trim().lines();
    for line in lines {
        if let Some(msg) = get_message(line) {
            if msg.is_empty() { continue; }
            // parse the first message.
            if map.contains_key(&msg) {
                log::error!("{msg} is duplicated in cobra output.");
            }
            current_message = msg;
        } else {
            if current_message.is_empty() { continue; }
            let indexes = line.match_indices(":").map(|(x, _)| x).collect::<Vec<_>>();
            if indexes.len() == 2
                && line[0..indexes[0]].trim().chars().all(|x| x.is_ascii_digit())
            {
                let _ = u32::from_str_radix(&line[0..indexes[0]].trim(), 10)
                    .ok()
                    .and_then(|_| {
                        let (begin, end) = get_range(&line[indexes[1] + 1..].trim())?;
                        let occurrence = Occurrence {
                            // TODO: can be risky in case of empty input
                            file: line[indexes[0] + 1..indexes[1]].trim().to_owned(),
                            begin,
                            end,
                        };
                        map.entry(current_message.clone())
                            .and_modify(|x| x.push(occurrence.clone()))
                            .or_insert(vec![occurrence]);
                        Some(())
                    });
            }
        }
    }
    Ok(map)
}

#[cfg(test)]
mod test_txt_parser {
    use std::collections::HashMap;

    use crate::result::Mark;

    use super::{parse, Occurrence};
    #[test]
    fn trivial() {
        let occurrence_map = parse(
            r#"
        === MESSAGE ===
        1: FILE_NAME:10..12 (2 lines)
          LINE_NO     CODE_SNIPPET
        2: FILE_NAME:13..13
            LINE_NO     CODE_SNIPPET
            LINE_NO     CODE_SNIPPET
        LINE_NO     CODE_SNIPPET
            LINE_NO     CODE_SNIPPET
        LINE_NO     CODE_SNIPPET
    3: FILE_NAME:14..15
        "#,
        )
        .unwrap();
        let expected_map = HashMap::from([(
            "MESSAGE".to_owned(),
            vec![
                Occurrence {
                    file: "FILE_NAME".to_owned(),
                    begin: Mark {
                        line: 10,
                        column: 0,
                    },
                    end: Mark {
                        line: 12,
                        column: 0,
                    },
                },
                Occurrence {
                    file: "FILE_NAME".to_owned(),
                    begin: Mark {
                        line: 13,
                        column: 0,
                    },
                    end: Mark {
                        line: 13,
                        column: 0,
                    },
                },
                Occurrence {
                    file: "FILE_NAME".to_owned(),
                    begin: Mark {
                        line: 14,
                        column: 0,
                    },
                    end: Mark {
                        line: 15,
                        column: 0,
                    },
                },
            ],
        )]);
        for (name, occurrences) in occurrence_map {
            assert_eq!(expected_map[&name], occurrences);
        }
    }

    #[should_panic]
    #[test]
    fn fail() {
        let occurrence_map = parse(
            r#"
        === MESSAGE ===
        1: FILE_NAME:10..12 (2 lines)
          LINE_NO     CODE_SNIPPET
        2: FILE_NAME:13...13
          LINE_NO     CODE_SNIPPET
        "#,// 3 dots instead of 2 will cause a panic
        )
        .unwrap();
        let expected_map = HashMap::from([(
            "MESSAGE".to_owned(),
            vec![
                Occurrence {
                    file: "FILE_NAME".to_owned(),
                    begin: Mark {
                        line: 10,
                        column: 0,
                    },
                    end: Mark {
                        line: 12,
                        column: 0,
                    },
                },
                Occurrence {
                    file: "FILE_NAME".to_owned(),
                    begin: Mark {
                        line: 13,
                        column: 0,
                    },
                    end: Mark {
                        line: 13,
                        column: 0,
                    },
                },
            ],
        )]);
        for (name, occurrences) in occurrence_map {
            assert_eq!(expected_map[&name], occurrences);
        }
    }
}