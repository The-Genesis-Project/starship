; ModuleID = 'helloworld.cpp'
source_filename = "helloworld.cpp"

@.str = private unnamed_addr constant [14 x i8] c"Hello World!\0A\00", align 1

define i32 @main() #0 {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0))
  ret i32 0
}

declare i32 @printf(i8*, ...) #1
