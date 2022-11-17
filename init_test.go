package main

import (
	"encoding/json"
	"log"
	"os"
	"path"
	"testing"
)

func Test_Main(t *testing.T) {
	currentDir, err := os.Getwd()
	if err != nil {
		currentDir = "/"
		t.Error()
	}

	CODE_PATH = getenv("CODE_PATH", path.Join(currentDir, "code"))
	TOOLBOX_PATH = getenv("TOOLBOX_PATH", path.Join(currentDir, "toolbox"))
	RESULT_PATH = path.Join(TOOLBOX_PATH, "/analysis_results.json")

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	// Parse analysis config
	analysisConfig := AnalysisConfig{}

	// Read configuration file
	analysisConfigBytes, err := os.ReadFile(path.Join(TOOLBOX_PATH, "analysis_config.json"))
	if err != nil {
		t.Error(ReadConfig{err.Error()})
	}

	err = json.Unmarshal(analysisConfigBytes, &analysisConfig)
	if err != nil {
		t.Error(JSONUnmarshal{err.Error()})
	}

	// To ensure same file is not added twice and to filter files with extension
	// *.cpp. *.c, *.hpp, *.h, *.cc
	selected := make(map[string]struct{})
	for _, file := range analysisConfig.Files {
		if hasExtension(file, VALID_EXTENSIONS...) {
			selected[file] = struct{}{}
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

	if len(analysisFiles) != 0 {
		cobraIssues, err := runCobra(analysisFiles)
		log.Printf("Found %v issues via cobra\n", len(cobraIssues))
		if err != nil {
			t.Error("Error running cobra:", err)
		} else {
			result.Issues = append(result.Issues, cobraIssues...)
		}
		if len(result.Issues) == 0 {
			t.Fatal("Didn't produce any errors")
		}
	}

	// Publish the result
	result.IsPassed = true
	result.ExtraData = ""

	resultJSON, err := json.Marshal(result)
	if err != nil {
		t.Error(err)
	}

	// Write result to a temporary path
	err = os.WriteFile(RESULT_PATH, resultJSON, 0o600)
	if err != nil {
		t.Error(err)
	}
}
