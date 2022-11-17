package main

import (
	"bufio"
	"encoding/json"
	"log"
	"os"
	"path"
	"path/filepath"
	"strings"
)

// FIXIT: Global variable which might change should be passed as arguments
var (
	CommonLocalIncs       = []string{}
	SuffixPathToParentMap = map[string][]string{}
)

type CompileCmd struct {
	Directory string   `json:"directory"`
	Arguments []string `json:"arguments"`
	File      string   `json:"file"`
}

func getSysIncFromClang(output string) []string {
	var (
		startMarker = "#include <...> search starts here:"
		endMarker   = "End of search list."
		includeList []string
		inRange     bool
	)

	for _, line := range strings.Split(output, "\n") {
		line = strings.TrimSpace(line)
		if line == endMarker {
			inRange = false
		} else if line == startMarker {
			inRange = true
		} else if inRange {
			line = strings.TrimSuffix(line, " (framework directory)")
			includeList = append(includeList, "-isystem"+line)
		}
	}

	return includeList
}

func getSrcfilesForAnalysis(conf *AnalysisConfig) []string {
	srcs := make([]string, 0, len(conf.Files))
	for _, file := range conf.Files {
		if hasExtension(file, CxxSrcExtns...) {
			srcs = append(srcs, file)
		}
	}
	return srcs
}

func isExpectedIncPath(fpath string) bool {
	result := false
	switch fpath {
	case
		path.Join(CodePath, "inc"),
		path.Join(CodePath, "include"),
		path.Join(CodePath, "src", "inc"),
		path.Join(CodePath, "src", "include"):
		result = true
	default:
		result = false
	}
	return result
}

func setCommonUsrIncs() {
	err := filepath.Walk(CodePath, func(fpath string, info os.FileInfo, err error) error {
		if info.IsDir() && isExpectedIncPath(fpath) {
			CommonLocalIncs = append(CommonLocalIncs, "-I"+fpath)
		}
		return nil
	})
	if err != nil {
		log.Println("Failed to build common user include list", err)
	}
	CommonLocalIncs = append(CommonLocalIncs, "-I"+CodePath)
}

// Returns the smaller path starting from the beginnging of the paths
func getCommonPath(p1, p2 string) string {
	pathList1 := strings.Split(p1, string(os.PathSeparator))
	pathList2 := strings.Split(p2, string(os.PathSeparator))
	// detect the smaller path
	shorterPathList := pathList1
	if len(pathList2) < len(shorterPathList) {
		shorterPathList = pathList2
	}
	prefix := ""
	for i := range shorterPathList {
		if pathList1[i] == pathList2[i] {
			prefix = filepath.Join(prefix, pathList1[i])
		} else {
			break
		}
	}
	return prefix
}

// Take a `#include "|<some/path/to/file"|>` and return some/path/to/file.
// Empty string is returned if the file path doesn't end with valid CXX extension
func getIncludedFile(incLine string) string {
	incLine = strings.TrimSpace(incLine)

	startMarker := ""
	endMarker := ""
	switch string(incLine[0]) {
	case "<":
		startMarker = "<"
		endMarker = ">"
	case "\"":
		startMarker = "\""
		endMarker = "\""
	default:
		return ""
	}

	tentaiveHeader := strings.TrimSpace(strings.TrimSuffix(strings.TrimPrefix(incLine, startMarker), endMarker))
	if !hasExtension(tentaiveHeader, CxxAllExtns...) {
		// Skip looking up for C++ standard header; this is based on the fact
		// that they don't have extenstion. Also this will not avoid unnecessary
		// look-up for C headers.
		return ""
	}
	return tentaiveHeader
}

// Returns the list of path declared using '#include "'
// Note: we only detect includes with single space between
// 'e' and '"'.
func getIncsFromFile(fpath string) []string {
	incs := []string{}
	fhandle, err := os.Open(fpath)
	if err != nil {
		log.Println("Failed to open the file:", fpath)
		return incs
	}
	defer func() {
		if err = fhandle.Close(); err != nil {
			log.Println("Failed to close the file:", fpath)
		}
	}()

	scanner := bufio.NewScanner(fhandle)

	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())

		if strings.HasPrefix(line, "#include") {
			incFilePath := getIncludedFile(strings.TrimPrefix(line, "#include"))
			if incFilePath != "" {
				incs = append(incs, incFilePath)
			}
		}
	}
	if err := scanner.Err(); err != nil {
		log.Println("Failed scanning -- looking for header files in", fpath, ":", err)
	}

	return incs
}

// Returns the path (out of all fpaths) is which "closest" to fpath. We define
// "closest" as the path which matches most when compared from the beginning of
// the paths.
func getClosestPath(fpath string, fpaths []string) string {
	if len(fpaths) < 1 {
		return ""
	}

	closestPath := fpaths[0]
	for i := 1; i < len(fpaths); i++ {
		currCmnPath := getCommonPath(fpath, fpaths[i])

		if len(currCmnPath) > len(closestPath) {
			closestPath = fpaths[i]
		}
	}
	log.Println("===== Making a choice")
	log.Println("Source path:", fpath)
	log.Println("Give header paths:", fpaths)
	log.Println("Closest path:", closestPath)
	log.Println()

	return closestPath
}

// For a given path (assuming the path is of header file) return the parent
// directory. If the file is present in multiple parent directory this function
// returns the one which is "closest" the given source file path (srcFilepath)
func getSuitableIncForHeader(headerPath string, srcFilepath string) string {
	incPath := ""
	parentDirs := SuffixPathToParentMap[headerPath]
	switch len(parentDirs) {
	case 0:
		incPath = ""
	case 1:
		incPath = parentDirs[0]
	default:
		// build absolute path of header files
		absHeaderPaths := make([]string, len(parentDirs))
		for i := range parentDirs {
			absHeaderPaths = append(absHeaderPaths, path.Join(parentDirs[i], headerPath))
		}
		// get the "closest" match of header file locations to include
		absIncPath := getClosestPath(srcFilepath, absHeaderPaths)
		// get the absolute path of "closest" header file
		incPath = strings.TrimSuffix(absIncPath, headerPath)
	}
	return incPath
}

func elementExist(element string, list []string) bool {
	for _, current := range list {
		if current == element {
			return true
		}
	}
	return false
}

// Returns a list of include which we assume are important for this file to compile.
// The includes
//   - general include folders line inc or include
//   - path to the header files defined in the source using ""
func getLocalIncsForFile(fpath string) []string {
	// Local include contain common include and file specific includes.
	// Also, if we have file specific include then do we need local includes?
	var localIncs []string

	// Get the common local include dir
	localIncs = append(localIncs, CommonLocalIncs...)

	if len(SuffixPathToParentMap) == 0 {
		// We don't have a map to find file-specific includes
		return localIncs
	}

	// get the list of relative path mention using #include directive with ""
	relIncsPaths := getIncsFromFile(fpath)
	for i := range relIncsPaths {
		// try and find a parent dir for the relative path
		incPath := getSuitableIncForHeader(relIncsPaths[i], fpath)

		// To avoid include failures due to #includes in header, we can perform the
		// same operation on include headers. This will involve calling the current
		// method on `incPath`. To handle infinite recursion, maintain a set of
		// seen files and on visiting the same header file, stop further calls to
		// `getLocalIncsForFile`.

		// Add -I only if the path exist
		if len(incPath) > 0 {
			incPathWithOption := "-I" + incPath
			if !elementExist(incPathWithOption, localIncs) {
				localIncs = append(localIncs, incPathWithOption)
			}
		}
	}

	return localIncs
}

// Builds a array of compile commands for every C/CPP implementation file
func buildCCInstances(conf *AnalysisConfig, sysIncs []string) []CompileCmd {
	files := getSrcfilesForAnalysis(conf)
	compileCmds := make([]CompileCmd, 0, len(files))

	// compiler invocation and system include is common for all files
	// we add g++ as if the source is built using g++
	buildCmd := append([]string{"g++", "-c", "-w"}, sysIncs...)

	for i := range files {
		// build and add include specific to file
		fileBuildCmd := make([]string, 0, len(buildCmd))
		fileBuildCmd = append(fileBuildCmd, buildCmd...)
		fileBuildCmd = append(fileBuildCmd, getLocalIncsForFile(files[i])...)
		cc := CompileCmd{
			Directory: path.Dir(files[i]),
			Arguments: append(fileBuildCmd, files[i]),
			File:      files[i],
		}
		compileCmds = append(compileCmds, cc)
	}

	switch len(compileCmds) {
	case 0:
		log.Println("Compile commands is empty")
	default:
		ccJSON, err := json.Marshal(compileCmds[0])
		if err == nil {
			log.Println("A sample compile command:", string(ccJSON))
		}
	}
	return compileCmds
}

func buildParentMap() error {
	rootPath := CodePath
	err := filepath.Walk(rootPath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			log.Printf("Failure while accessing a path %q: %v\n", path, err)
			return err
		}

		if !info.IsDir() {
			if !hasExtension(path, CxxAllExtns...) {
				return nil
			}
			// Found a file, check extension

			relativePath, err := filepath.Rel(rootPath, path)
			if err != nil {
				log.Println("Internal bug -- path", path, "should've been inside codePath", rootPath)
				return err
			}

			splitPath := strings.Split(relativePath, string(os.PathSeparator))

			for i := len(splitPath) - 1; i >= 0; i-- {
				suffixSlice := splitPath[i:]
				suffix := filepath.Join(suffixSlice...)

				parents := SuffixPathToParentMap[suffix]
				newParent := strings.TrimSuffix(path, suffix)
				// there might be a better way to do this! goal is to avoid duplicate parents
				if !elementExist(newParent, parents) {
					// the suffix have a parent list, and we have a new parent
					SuffixPathToParentMap[suffix] = append(SuffixPathToParentMap[suffix], newParent)
				}
			}
		}
		return nil
	})
	if err != nil {
		log.Println("Failed to build file path suffix to parent dir map:", err)
		return err
	}
	return nil
}

func initCCBuilder() {
	// build a hash map from every suffix for every file to its parent directory
	buildParentMap()
	// make sure common include path by user is empty
	CommonLocalIncs = []string{}
	// fetch includes with common names
	setCommonUsrIncs()
}

func getSysIncs() []string {
	cppCompiler := "g++"
	args := []string{"-Wp,-v", "-x", "c++", "-", "-fsyntax-only"}
	// We don't have to run the cmd in ToolboxPath, but runCmd don't have
	// an optional argument.
	stdout, stderr, err := runCmd(cppCompiler, args, ToolboxPath)
	if err != nil {
		log.Println("g++ invokation failed")
		log.Println("stdout for g++: ", string(stdout))
		log.Println("stderr for g++: ", string(stderr))
		return []string{}
	}

	return getSysIncFromClang(string(stderr))
}

// Function for debug use
func printHashMap() error {
	prettyContent, err := json.MarshalIndent(SuffixPathToParentMap, "", "	")

	if err != nil {
		log.Println("Failed to convert SuffixPathToParentMap to json:", err)
		return err
	}
	log.Println(string(prettyContent))
	return nil
}

func buildCompileCmds(conf *AnalysisConfig) error {
	initCCBuilder()

	sysInc := getSysIncs()
	compileCmds := buildCCInstances(conf, sysInc)

	ccJSON, err := json.MarshalIndent(compileCmds, "", "	")
	if err != nil {
		return err
	}

	err = os.WriteFile(CompileCommandFilePath, ccJSON, 0o644)
	if err != nil {
		return err
	}

	return nil
}
