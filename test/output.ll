; ModuleID = 'main.rk'
source_filename = "main.rk"

@0 = private constant [1 x i8] zeroinitializer

define void @main(i32 %0, i32 %1) {
entry:
  %2 = call double @rint(i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0))
}

declare double @rint(double)
