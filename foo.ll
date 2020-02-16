; ModuleID = 'foo.ll'
source_filename = "foo.ll"
target datalayout = "e-m:e-p:32:32-i64:64-n32:64-S128"
target triple = "wasm32-unknown-wasi"

%swift.method_descriptor = type { i32, i32 }
%swift.type = type { i32 }
%swift.opaque = type opaque
%swift.type_metadata_record = type { i32 }
%swift.refcounted = type { %swift.type*, i32 }
%T3foo10SwiftClassC = type <{ %swift.refcounted }>
%swift.metadata_response = type { %swift.type*, i32 }

@"$sBoWV" = external global i8*, align 4
@0 = private constant [4 x i8] c"foo\00"
@"$s3fooMXM" = linkonce_odr hidden constant <{ i32, i32, i32 }> <{ i32 0, i32 0, i32 ptrtoint ([4 x i8]* @0 to i32) }>, section ".rodata", align 4
@1 = private constant [11 x i8] c"SwiftClass\00"
@"$s3foo10SwiftClassCMn" = hidden constant <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }> <{ i32 -2147483568, i32 ptrtoint (<{ i32, i32, i32 }>* @"$s3fooMXM" to i32), i32 ptrtoint ([11 x i8]* @1 to i32), i32 ptrtoint (%swift.metadata_response (i32)* @"$s3foo10SwiftClassCMa" to i32), i32 ptrtoint ({ i32, i32, i16, i16, i32 }* @"$s3foo10SwiftClassCMF" to i32), i32 0, i32 2, i32 14, i32 1, i32 0, i32 13, i32 13, i32 1, %swift.method_descriptor { i32 1, i32 0 } }>, section ".rodata", align 4
@"$s3foo10SwiftClassCML" = internal global %swift.type* null, align 4
@"$s3foo10SwiftClassCMf" = internal global <{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }> <{ i8* null, i8** @"$sBoWV", i32 0, %swift.type* null, %swift.opaque* null, %swift.opaque* null, i32 1, i32 2, i32 0, i32 8, i16 3, i16 0, i32 64, i32 8, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>* @"$s3foo10SwiftClassCMn", i8* null, i8* bitcast (void ()* @swift_deletedMethodError to i8*) }>, align 4
@"got.$s3foo10SwiftClassCMn" = private unnamed_addr constant <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>* @"$s3foo10SwiftClassCMn"
@"symbolic _____ 3foo10SwiftClassC" = linkonce_odr hidden constant <{ i8, i32, i8 }> <{ i8 2, i32 ptrtoint (<{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>** @"got.$s3foo10SwiftClassCMn" to i32), i8 0 }>, section "swift5_typeref", align 2
@"$s3foo10SwiftClassCMF" = internal constant { i32, i32, i16, i16, i32 } { i32 ptrtoint (<{ i8, i32, i8 }>* @"symbolic _____ 3foo10SwiftClassC" to i32), i32 0, i16 1, i16 12, i32 0 }, section "swift5_fieldmd", align 4
@"\01l_type_metadata_table" = private constant [1 x %swift.type_metadata_record] [%swift.type_metadata_record { i32 add (i32 ptrtoint (<{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>** @"got.$s3foo10SwiftClassCMn" to i32), i32 1) }], section "swift5_type_metadata", align 4
@__swift_reflection_version = linkonce_odr hidden constant i16 3
@_swift1_autolink_entries = private constant [12 x i8] c"-lswiftCore\00", section ".swift1_autolink_entries", align 4
@llvm.used = appending global [7 x i8*] [i8* bitcast (void ()* @closureToConvert to i8*), i8* bitcast (void ()* @testConvertFunc to i8*), i8* bitcast ({ i8*, %swift.refcounted* } (%T3foo10SwiftClassC*)* @partial_apply_class to i8*), i8* bitcast ({ i32, i32, i16, i16, i32 }* @"$s3foo10SwiftClassCMF" to i8*), i8* bitcast ([1 x %swift.type_metadata_record]* @"\01l_type_metadata_table" to i8*), i8* bitcast (i16* @__swift_reflection_version to i8*), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @_swift1_autolink_entries, i32 0, i32 0)], section "llvm.metadata", align 4

@"$s3foo10SwiftClassCACycfCTq" = hidden alias %swift.method_descriptor, getelementptr inbounds (<{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>* @"$s3foo10SwiftClassCMn", i32 0, i32 13)
@"$s3foo10SwiftClassCN" = hidden alias %swift.type, bitcast (i32* getelementptr inbounds (<{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>, <{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>* @"$s3foo10SwiftClassCMf", i32 0, i32 2) to %swift.type*)
define swiftcc void @closureToConvert() #0 {
entry:
  ret void
}
define swiftcc void @testConvertFunc() #0 {
entry:
  call swiftcc void @"$sTA"(%swift.refcounted* swiftself bitcast (void ()* @closureToConvert to %swift.refcounted*))
  ret void
}
define internal void @"$sTA"(%swift.refcounted* swiftself %0) {
entry:
  %1 = bitcast %swift.refcounted* %0 to void ()*
  call swiftcc void %1()
  ret void
}
; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #1
define swiftcc { i8*, %swift.refcounted* } @partial_apply_class(%T3foo10SwiftClassC* %0) #0 {
entry:
  %1 = bitcast %T3foo10SwiftClassC* %0 to %swift.refcounted*
  %2 = insertvalue { i8*, %swift.refcounted* } { i8* bitcast (void (%swift.refcounted*)* @"$s28partially_applyable_to_classTA" to i8*), %swift.refcounted* undef }, %swift.refcounted* %1, 1
  ret { i8*, %swift.refcounted* } %2
}
declare swiftcc void @partially_applyable_to_class(%T3foo10SwiftClassC*) #0
define internal swiftcc void @"$s28partially_applyable_to_classTA"(%swift.refcounted* swiftself %0) #0 {
entry:
  %1 = bitcast %swift.refcounted* %0 to %T3foo10SwiftClassC*
  tail call swiftcc void @partially_applyable_to_class(%T3foo10SwiftClassC* %1)
  ret void
}
; Function Attrs: noinline nounwind readnone
define hidden swiftcc %swift.metadata_response @"$s3foo10SwiftClassCMa"(i32 %0) #2 {
entry:
  %1 = load %swift.type*, %swift.type** @"$s3foo10SwiftClassCML", align 4
  %2 = icmp eq %swift.type* %1, null
  br i1 %2, label %cacheIsNull, label %cont

cacheIsNull:                                      ; preds = %entry
  store atomic %swift.type* bitcast (i32* getelementptr inbounds (<{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>, <{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>* @"$s3foo10SwiftClassCMf", i32 0, i32 2) to %swift.type*), %swift.type** @"$s3foo10SwiftClassCML" release, align 4
  br label %cont

cont:                                             ; preds = %cacheIsNull, %entry
  %3 = phi %swift.type* [ %1, %entry ], [ bitcast (i32* getelementptr inbounds (<{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>, <{ i8*, i8**, i32, %swift.type*, %swift.opaque*, %swift.opaque*, i32, i32, i32, i32, i16, i16, i32, i32, <{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>*, i8*, i8* }>* @"$s3foo10SwiftClassCMf", i32 0, i32 2) to %swift.type*), %cacheIsNull ]
  %4 = insertvalue %swift.metadata_response undef, %swift.type* %3, 0
  %5 = insertvalue %swift.metadata_response %4, i32 0, 1
  ret %swift.metadata_response %5
}
; Function Attrs: nounwind
declare void @swift_deletedMethodError() #3

attributes #0 = { "frame-pointer"="all" "target-cpu"="generic" }
attributes #1 = { cold noreturn nounwind }
attributes #2 = { noinline nounwind readnone "frame-pointer"="none" "target-cpu"="generic" }
attributes #3 = { nounwind }

!swift.module.flags = !{!0}
!llvm.asan.globals = !{!1, !2, !3, !4, !5, !6, !7}
!llvm.linker.options = !{}
!llvm.module.flags = !{!8, !9, !10, !11}

!0 = !{!"standard-library", i1 false}
!1 = !{<{ i32, i32, i32 }>* @"$s3fooMXM", null, null, i1 false, i1 true}
!2 = !{<{ i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %swift.method_descriptor }>* @"$s3foo10SwiftClassCMn", null, null, i1 false, i1 true}
!3 = !{<{ i8, i32, i8 }>* @"symbolic _____ 3foo10SwiftClassC", null, null, i1 false, i1 true}
!4 = !{{ i32, i32, i16, i16, i32 }* @"$s3foo10SwiftClassCMF", null, null, i1 false, i1 true}
!5 = !{[1 x %swift.type_metadata_record]* @"\01l_type_metadata_table", null, null, i1 false, i1 true}
!6 = !{[12 x i8]* @_swift1_autolink_entries, null, null, i1 false, i1 true}
!7 = !{[7 x i8*]* @llvm.used, null, null, i1 false, i1 true}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{i32 4, !"Objective-C Garbage Collection", i32 84018944}
!11 = !{i32 1, !"Swift Version", i32 7}
