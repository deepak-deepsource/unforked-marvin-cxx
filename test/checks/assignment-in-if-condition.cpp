void f(int arg) {
  int f = 3;
  // [CXX-W2000]: 8 "Found assignment `f = arg` in `if` condition"
  if ((f = arg) || (f == (arg + 1)))
  {
    f = 5;
  }
}

void f1(int arg) {
  int f = 3;
  // [CXX-W2000]: 22 "Found assignment `f = (arg + 1)` in `if` condition"
  if ((f == arg) || (f = (arg + 1)))
  {
    f = 5;
  }
}

void f2(int arg) {
  int f = 3;
  // [CXX-W2000]: 7 "Found assignment `f = arg` in `if` condition"
  if (f = arg)
  {
    f = 5;
  }
}

volatile int v = 32;

void f3(int arg) {
  int f = 3;
  // [CXX-W2000]: 40 "Found assignment `f = v` in `if` condition"
  if ((f == arg) || ((arg + 6 < f) && (f = v)))
  {
    f = 5;
  }
}

void f4(int arg) {
  int f = 3;
  // [CXX-W2000]: 41 "Found assignment `f = v` in `if` condition"
  if ((f == arg) || ((arg + 6 < f) && ((f = v) || (f < 8))))
  {
    f = 5;
  } else if ((arg + 8 < f) && ((f = v) || (f < 8))) // [CXX-W2000]: 33 "Found assignment `f = v` in `if` condition"
  {
    f = 6;
  }
}

class BrokenOperator {
public:
  int d = 0;
  int operator=(const int &val) {
    d = val + 1;
    return d;
  }
};

void f5(int arg) {
  BrokenOperator bo;
  int f = 3;
  bo = f;
  if (bo.d == 3) {
    f = 6;
  }
  // [CXX-W2000]: 7 "Found assignment `bo = 3` in `if` condition"
  if (bo = 3)
  {
    f = 7;
  }
  // [CXX-W2000]: 22 "Found assignment `bo = 6` in `if` condition"
  if ((arg == 3) || (bo = 6))
  {
    f = 8;
  }
  v = f;
}

// Tests that shouldn't trigger warnings.
void awesome_f2(int arg) {
  int f = 3;
  if ((f == arg) || (f == (arg + 1))) {
    f = 5;
  }
}

void awesome_f3(int arg) {
  int f = 3;
  if (f == arg) {
    f = 5;
  }
}

void awesome_f4(int arg) {
  int f = 3;
  if ((f == arg) || ((arg + 6 < f) && ((f == v) || (f < 8)))) {
    f = 5;
  }
}

template <typename Func> bool exec(Func F) { return F(); }

void lambda_if() {
  int X;
  if ([&X] {
        X = 5;
        return true;
      }()) {
  }

  if (exec([&] {
        X = 5;
        return true;
      })) {
  }
}
