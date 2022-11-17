package main

import (
	"encoding/json"
	"log"
	"os"
	"path"
	"strings"
	"time"

	"github.com/getsentry/sentry-go"
)

var (
	ToolboxPath            = getenv("TOOLBOX_PATH", "/toolbox")
	CodePath               = getenv("CODE_PATH", "/code")
	CompileCommandFilePath = path.Join(CodePath, "compile_commands.json")
	ResultPath             = path.Join(ToolboxPath, "analysis_results.json")
	ConfigPath             = path.Join(ToolboxPath, "analysis_config.json")
	CxxAllExtns            = []string{".cc", ".cpp", ".c", ".hpp", ".h"}
	CxxSrcExtns            = []string{".cc", ".cpp", ".c"}
	NonCExtns              = []string{".cpp", ".cc", ".hpp"}
)

type Location struct {
	Path     string `json:"path"`
	Position struct {
		Begin struct {
			Line   int `json:"line"`
			Column int `json:"column"`
		} `json:"begin"`
		End struct {
			Line   int `json:"line"`
			Column int `json:"column"`
		} `json:"end"`
	} `json:"position"`
}

type Meta struct{}

type Issue struct {
	IssueCode string   `json:"issue_code"`
	IssueText string   `json:"issue_text"`
	Location  Location `json:"location"`
}
type AnalysisError struct {
	HMessage string `json:"hmessage"`
	Level    int    `json:"level"`
}

type Result struct {
	Issues    []Issue         `json:"issues"`
	Metrics   []Metric        `json:"metrics"`
	IsPassed  bool            `json:"is_passed"`
	Errors    []AnalysisError `json:"errors"`
	ExtraData interface{}     `json:"extra_data"`
}

type AnalysisConfig struct {
	Files           []string `json:"files"`
	ExcludePatterns []string `json:"exclude_patterns"`
	TestFiles       []string `json:"test_files"`
	AnalyzerMeta    struct {
		Meta       Meta   `toml:"meta,omitempty" json:"meta,omitempty"`
		Name       string `toml:"name" json:"name"`
		CppVersion string `toml:"cpp_version,omitempty" json:"cpp_version,omitempty"`
		Enabled    bool   `toml:"enabled" json:"enabled"`
	} `json:"analyzer_meta"`
}

type Namespace struct {
	Key   string  `json:"key"`
	Value float64 `json:"value"`
}

type Metric struct {
	MetricCode string      `json:"metric_code"`
	Namespaces []Namespace `json:"namespaces"`
}

func elog(err error, msg string) {
	if err != nil {
		log.Printf("%s: %v", msg, err)
	}
}

func init() {
	log.SetFlags(log.LstdFlags | log.Lshortfile)
	elog(sentry.Init(sentry.ClientOptions{
		Dsn:              os.Getenv("SENTRY_DSN"),
		Environment:      "marvin-cxx",
		AttachStacktrace: true,
	}), "sentry init failed")
}

func hasExtension(src string, extensions ...string) bool {
	for i := range extensions {
		if strings.HasSuffix(src, extensions[i]) {
			return true
		}
	}
	return false
}

func getenv(key, fallback string) string {
	val := os.Getenv(key)
	if val == "" {
		return fallback
	}
	return val
}

func runCobra(files []string) ([]Issue, error) {
	pathToCobra := path.Join(ToolboxPath, "cobra", "cobra")
	filesSpaced := strings.Join(files, " ")

	stdout, stderr, err := runCmd(path.Join(ToolboxPath, "cobrarust"), []string{pathToCobra, filesSpaced}, ToolboxPath)
	if err != nil {
		return nil, err
	}

	log.Println("cobrarust stderr:\n", string(stderr))
	log.Println("cobrarust stdout:\n", string(stdout))
	return buildIssuesFromStdout(stdout)
}

func buildIssuesFromStdout(b []byte) ([]Issue, error) {
	var issues []Issue
	if err := json.Unmarshal(b, &issues); err != nil {
		return nil, err
	}
	issues_to_report := make([]Issue, 0, len(issues))
	for i := range issues {
		if issues[i].Location.Path == "" {
			continue
		}
		// Example path: /code/deps/jemalloc/test/unit/log.c
		// Expected: deps/jemalloc/test/unit/log.c
		if issues[i].Location.Path[0] != '/' {
			issues[i].Location.Path = "/" + issues[i].Location.Path
		}

		if !strings.HasPrefix(issues[i].Location.Path, CodePath) {
			// skip issue from non-user code like standard headers
			continue
		}
		if strings.HasSuffix(CodePath, "/") {
			issues[i].Location.Path = strings.TrimPrefix(issues[i].Location.Path, CodePath)
		} else {
			issues[i].Location.Path = strings.TrimPrefix(issues[i].Location.Path, CodePath+"/")
		}
		issues_to_report = append(issues_to_report, issues[i])
	}

	return issues_to_report, nil
}

func isNotCpp(file string) bool {
	// A non-existing/text file is also not C++, which is correct. But then
	// it's also not C, and we don't want any analysis, making this check
	// less reliable. We filter out files based on the extension, so this
	// logic should work for now.
	if hasExtension(file, NonCExtns...) {
		return false
	}
	_, _, err := runCmd(path.Join(ToolboxPath, "is_cpp"), []string{file}, ToolboxPath)
	if err != nil {
		return true
	} else {
		return false
	}
}

func runLibtoolingBasedAnalyzer(files []string) ([]Issue, error) {
	// the command to run the analyser
	analyzerPath := path.Join(ToolboxPath, "cxxengine")

	// build argument to analyser
	analyzerArgs := make([]string, 0, len(files)+2)
	analyzerArgs = append(analyzerArgs, "-p", CompileCommandFilePath)
	analyzerArgs = append(analyzerArgs, files...)

	stdout, stderr, err := runCmd(analyzerPath, analyzerArgs, ToolboxPath)
	log.Println("Done running LTA")
	if err != nil {
		log.Println("WARNING: LTA returned an error", err)
		log.Println("LTA stderr: ", string(stderr))
	}
	log.Println("LTA stdout: ", string(stdout))
	if string(stdout) != "" {
		return buildIssuesFromStdout(stdout)
	} else {
		log.Println("WARNING: LTA stdout is empty")
		return nil, err
	}
}

func main() {
	defer sentry.Flush(2 * time.Second)

	// Parse analysis config
	analysisConfig := AnalysisConfig{}

	// Read configuration file
	analysisConfigBytes, err := os.ReadFile(ConfigPath)
	if err != nil {
		log.Println(err)
		sentry.CaptureException(ReadConfig{err.Error()})
	}

	err = json.Unmarshal(analysisConfigBytes, &analysisConfig)
	if err != nil {
		log.Println(err)
		sentry.CaptureException(JSONUnmarshal{err.Error()})
	}

	// Filter files with extensions: ".cpp", ".c", ".cc", ".hpp", ".h" and hold
	// unique filepaths.
	selected := make(map[string]struct{})
	for _, file := range analysisConfig.Files {
		if hasExtension(file, CxxAllExtns...) {
			selected[file] = struct{}{}
		}
	}

	cAnalysisFiles := make([]string, 0, len(selected))
	for file := range selected {
		if isNotCpp(file) {
			cAnalysisFiles = append(cAnalysisFiles, file)
		}
	}

	analysisFiles := make([]string, 0, len(selected))
	for k := range selected {
		analysisFiles = append(analysisFiles, k)
	}

	// Analysis results
	result := Result{}
	result.Issues = []Issue{}
	result.Metrics = []Metric{}
	result.Errors = []AnalysisError{}

	// Cobra pass
	if len(cAnalysisFiles) != 0 {
		cobraIssues, err := runCobra(cAnalysisFiles)
		if err != nil {
			log.Println("Error running cobrarust:", err)
			sentry.CaptureException(Cobra{err.Error()})
		} else {
			log.Println("Issues found via cobra:", len(cobraIssues))
			result.Issues = append(result.Issues, cobraIssues...)
		}
	}

	// LTA Pass
	if len(analysisFiles) != 0 {
		err = buildCompileCmds(&analysisConfig)
		if err != nil {
			log.Println("Error building compile commands:", err)
		} else {
			// FIXIT: Code duplication here
			srcFiles := []string{}
			for i := range analysisFiles {
				if hasExtension(analysisFiles[i], CxxSrcExtns...) {
					srcFiles = append(srcFiles, analysisFiles[i])
				}
			}
			log.Println("Running LTA on ", len(srcFiles), " files")
			ltaIssues, err := runLibtoolingBasedAnalyzer(srcFiles)
			if err != nil {
				log.Printf("Error running LTA: %v, issues: %v\n", err, ltaIssues)
			} else {
				log.Println("Issues found via LTA:", len(ltaIssues))
				result.Issues = append(result.Issues, ltaIssues...)
			}
		}
	}

	// Publish the result
	result.IsPassed = true
	result.ExtraData = ""

	resultJSON, err := json.MarshalIndent(result, "", "  ")
	if err != nil {
		log.Println(err)
	}

	// Write result to a temporary path
	err = os.WriteFile(ResultPath, resultJSON, 0o600)
	if err != nil {
		log.Println(err)
	}
}
