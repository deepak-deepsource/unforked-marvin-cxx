package main

import (
	"bytes"
	"log"
	"os"
	"os/exec"
	"time"
)

func runCmd(command string, args []string, cmdDir string, env ...string) ([]byte, []byte, error) {
	cmd := exec.Command(command, args...)
	cmd.Dir = cmdDir
	cmd.Env = append(os.Environ(), env...)

	var stdoutBuf, stderrBuf bytes.Buffer
	cmd.Stdout, cmd.Stderr = &stdoutBuf, &stderrBuf

	log.Println("[EXEC] Command:", cmd.Path)
	log.Println("[EXEC] Args:", cmd.Args)
	log.Println("[EXEC] Dir: ", cmd.Dir)
	log.Println("[EXEC] Env: ", env)

	t0 := time.Now()
	err := cmd.Run()
	log.Println("[EXEC] Time Taken:", time.Since(t0)/time.Millisecond)
	log.Println("[EXEC] Command End:", cmd.Path)

	return stdoutBuf.Bytes(), stderrBuf.Bytes(), err
}
