; ModuleID = 'main.rk'
source_filename = "main.rk"

@0 = private unnamed_addr constant [14 x i8] c"Hello, World\0A\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"test\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"\\n\00", align 1

define [5 x i8]* @main(i32 %0, i32 %1) {
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @0, i32 0, i32 0))
  ret [5 x i8]* @1
}

declare i32 @printf(i8*, ...)

define [3 x i8]* @newline() {
  ret [3 x i8]* @2
}
