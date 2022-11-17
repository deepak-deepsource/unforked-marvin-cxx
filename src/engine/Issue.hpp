#include <string>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace MCxxIssue {
struct FilePoint {
  unsigned int Line;
  unsigned int Column;
  FilePoint(unsigned int Line, unsigned int Column)
      : Line{Line}, Column{Column} {}
  bool IsInvalid(void) { return Line == 0 || Column == 0; }
};

inline void to_json(json &j, const FilePoint &FP) {
  j = json{{"line", FP.Line}, {"column", FP.Column}};
}

inline void from_json(const json &j, FilePoint &FP) {
  j.at("line").get_to(FP.Line);
  j.at("column").get_to(FP.Column);
}

struct FileRange {
  FilePoint Begin;
  FilePoint End;
  FileRange(FilePoint Begin, FilePoint End) : Begin{Begin}, End{End} {}
  bool IsInvalid(void) { return Begin.IsInvalid() || End.IsInvalid(); }
};

inline void to_json(json &j, const FileRange &FR) {
  j = json{{"begin", FR.Begin}, {"end", FR.End}};
}

inline void from_json(const json &j, FileRange &FR) {
  j.at("begin").get_to(FR.Begin);
  j.at("end").get_to(FR.End);
}

struct Location {
  std::string Path;
  FileRange Position;
  Location(std::string Path, FileRange Position)
      : Path{Path}, Position{Position} {}

  bool IsInvalid(void) { return Path.empty() || Position.IsInvalid(); }
};

inline void to_json(json &j, const Location &Loc) {
  j = json{{"path", Loc.Path}, {"position", Loc.Position}};
}

inline void from_json(const json &j, Location &Loc) {
  j.at("path").get_to(Loc.Path);
  j.at("position").get_to(Loc.Position);
}

struct Issue {
  std::string Code;
  std::string Message;
  Location Loc;
  Issue();
  Issue(std::string Code, std::string Message, Location Loc)
      : Code{Code}, Message{Message}, Loc{Loc} {}
};

inline void to_json(json &j, const Issue &AnIssue) {
  j = json{{"issue_code", AnIssue.Code},
           {"issue_text", AnIssue.Message},
           {"location", AnIssue.Loc}};
}

inline void from_json(const json &j, Issue &I) {
  j.at("issue_code").get_to(I.Code);
  j.at("issue_text").get_to(I.Message);
  j.at("location").get_to(I.Loc);
}
} // namespace MCxxIssue
