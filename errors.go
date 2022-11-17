package main

// NOTE(SS): This file is a workaround to make sentry print distinguishable names
// (slack notifs).
//
// Taken from sentry-go@v0.11.0/client.go
// ```go
//  event.Exception = append(event.Exception, Exception{
//      Value:      err.Error(),
// 	    Type:       reflect.TypeOf(err).String(),
// 	    Stacktrace: ExtractStacktrace(err),
//  })
// ```
// As we cannot change the implementation of sentry-go's, we would go for a workaround
// which is even correct — no hacks!

type Cobra struct {
	err string
}

func (e Cobra) Error() string {
	return e.err
}

type MarvinCpp struct {
	err string
}

func (e MarvinCpp) Error() string {
	return e.err
}

type PublishResult struct {
	err string
}

func (e PublishResult) Error() string {
	return e.err
}

type ReadConfig struct {
	err string
}

func (e ReadConfig) Error() string {
	return e.err
}

type JSONUnmarshal struct {
	err string
}

func (e JSONUnmarshal) Error() string {
	return e.err
}
