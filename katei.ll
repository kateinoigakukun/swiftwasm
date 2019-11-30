; ModuleID = 'katei.ll'
source_filename = "katei.ll"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%swift.type = type { i64 }
%swift.full_type = type { i8**, %swift.type }
%swift.type_metadata_record = type { i64 }
%Any = type { [24 x i8], %swift.type* }
%T5katei3BoxV = type opaque
%swift.opaque = type opaque
%swift.type_descriptor = type opaque
%swift.metadata_response = type { %swift.type*, i64 }

@"$sSiN" = external global %swift.type, align 8
@"symbolic _________ySiG 5katei3BoxV" = linkonce_odr hidden constant <{ i8, i64, [4 x i8], i8 }> <{ i8 1, i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to i64), [4 x i8] c"ySiG", i8 0 }>, section "__DATA,__swift5_typeref, regular, no_dead_strip", align 2
@"$s5katei3BoxVySiGMD" = linkonce_odr hidden global { i64, i32 } { i64 ptrtoint (<{ i8, i64, [4 x i8], i8 }>* @"symbolic _________ySiG 5katei3BoxV" to i64), i32 -13 }, align 8
@"$sypN" = external global %swift.full_type
@"$sytWV" = external global i8*, align 8
@"got.$sytWV" = private unnamed_addr constant i8** @"$sytWV"
@0 = private constant [6 x i8] c"katei\00"
@"$s5kateiMXM" = linkonce_odr hidden constant <{ i32, i64, i64 }> <{ i32 0, i64 0, i64 ptrtoint ([6 x i8]* @0 to i64) }>, section "__DATA,__const", align 4
@1 = private constant [4 x i8] c"Box\00"
@"$s5katei3BoxVMI" = internal global [16 x i8*] zeroinitializer, align 8
@"$s5katei3BoxVMn" = constant <{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>
<{ 
  i32 209,
  i64 ptrtoint (<{ i32, i64, i64 }>* @"$s5kateiMXM" to i64),
  i64 ptrtoint ([4 x i8]* @1 to i64),
  i64 ptrtoint (%swift.metadata_response (i64, %swift.type*)* @"$s5katei3BoxVMa" to i64),
  i64 ptrtoint ({ i64, i32, i16, i16, i32 }* @"$s5katei3BoxVMF" to i64),
  i32 0,
  i32 3,
  i64 ptrtoint ([16 x i8*]* @"$s5katei3BoxVMI" to i64),
  i64 ptrtoint (<{ i64, i32, i32, i64 }>* @"$s5katei3BoxVMP" to i64),
  i16 1, i16 0, i16 1, i16 0,
  i8 -128, i8 0, i8 0, i8 0
}>, section "__DATA,__const", align 4
@"$s5katei3BoxVMP" = internal constant <{ i64, i32, i32, i64 }>
<{
  i64 ptrtoint (%swift.type* (%swift.type_descriptor*, i8**, i8*)* @"$s5katei3BoxVMi" to i64),
  i32 0,
  i32 1073741824,
  i64 add (i64 ptrtoint (i8*** @"got.$sytWV" to i64), i64 1)
}>, align 8
@"symbolic _________ 5katei3BoxV" = linkonce_odr hidden constant <{ i8, i64, i8 }> <{ i8 1, i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to i64), i8 0 }>, section "__DATA,__swift5_typeref, regular, no_dead_strip", align 2
@"$s5katei3BoxVMF" = internal constant { i64, i32, i16, i16, i32 } { i64 ptrtoint (<{ i8, i64, i8 }>* @"symbolic _________ 5katei3BoxV" to i64), i32 0, i16 0, i16 12, i32 0 }, section "__DATA,__swift5_fieldmd, regular, no_dead_strip", align 4
@"_swift_FORCE_LOAD_$_swiftCompatibility50_$_katei" = weak_odr hidden constant void ()* @"_swift_FORCE_LOAD_$_swiftCompatibility50"
@"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements_$_katei" = weak_odr hidden constant void ()* @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements"
@"\01l_type_metadata_table" = private constant [1 x %swift.type_metadata_record] [%swift.type_metadata_record { i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to i64) }], section "__DATA, __swift5_types, regular, no_dead_strip", align 4
@__swift_reflection_version = linkonce_odr hidden constant i16 3
@llvm.used = appending global [9 x i8*] [i8* bitcast (void ()* @"$s5katei3BoxVAASiRszlE3fooyyFZ" to i8*), i8* bitcast (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to i8*), i8* bitcast (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to i8*), i8* bitcast (%swift.metadata_response (i64, %swift.type*)* @"$s5katei3BoxVMa" to i8*), i8* bitcast ({ i64, i32, i16, i16, i32 }* @"$s5katei3BoxVMF" to i8*), i8* bitcast (void ()** @"_swift_FORCE_LOAD_$_swiftCompatibility50_$_katei" to i8*), i8* bitcast (void ()** @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements_$_katei" to i8*), i8* bitcast ([1 x %swift.type_metadata_record]* @"\01l_type_metadata_table" to i8*), i8* bitcast (i16* @__swift_reflection_version to i8*)], section "llvm.metadata", align 8

define i32 @main(i32, i8**) #0 {
entry:
  %2 = alloca %Any, align 8
  %3 = bitcast i8** %1 to i8*
  %4 = bitcast %Any* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* %4)
  call swiftcc void @"$s5katei3BoxVACyxGycfC"(%swift.type* @"$sSiN")
  %5 = call %swift.type* @__swift_instantiateConcreteTypeFromMangledName({ i64, i32 }* @"$s5katei3BoxVySiGMD") #4
  %6 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 1
  store %swift.type* %5, %swift.type** %6, align 8
  %7 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 0
  %8 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 0
  %9 = bitcast [24 x i8]* %8 to %T5katei3BoxV*
  %10 = bitcast %Any* %2 to %swift.opaque*
  %11 = call i1 @swift_dynamicCast(%swift.opaque* undef, %swift.opaque* %10, %swift.type* getelementptr inbounds (%swift.full_type, %swift.full_type* @"$sypN", i32 0, i32 1), %swift.type* %5, i64 7) #5
  %12 = bitcast %Any* %2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 32, i8* %12)
  call swiftcc void @"$s5katei3BoxVAASiRszlE3fooyyFZ"()
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

define hidden swiftcc void @"$s5katei3BoxVACyxGycfC"(%swift.type* %T) #0 {
entry:
  %T1 = alloca %swift.type*, align 8
  store %swift.type* %T, %swift.type** %T1, align 8
  ret void
}

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #2

; Function Attrs: noinline nounwind readnone
define linkonce_odr hidden %swift.type* @__swift_instantiateConcreteTypeFromMangledName({ i64, i32 }*) #3 {
entry:
  %1 = bitcast { i64, i32 }* %0 to i64*
  %2 = load i64, i64* %1, align 8
  %3 = inttoptr i64 %2 to i8*
  %4 = getelementptr inbounds { i64, i32 }, { i64, i32 }* %0, i32 0, i32 1
  %5 = load atomic i32, i32* %4 monotonic, align 8
  %6 = sext i32 %5 to i64
  %7 = icmp slt i64 %6, 0
  %8 = call i1 @llvm.expect.i1(i1 %7, i1 false)
  br i1 %8, label %12, label %9

9:                                                ; preds = %12, %entry
  %10 = phi i64 [ %2, %entry ], [ %15, %12 ]
  %11 = inttoptr i64 %10 to %swift.type*
  ret %swift.type* %11

12:                                               ; preds = %entry
  %13 = sub i64 0, %6
  %14 = call swiftcc %swift.type* @swift_getTypeByMangledNameInContext(i8* %3, i64 %13, %swift.type_descriptor* null, i8** null) #4
  %15 = ptrtoint %swift.type* %14 to i64
  store atomic i64 %15, i64* %1 monotonic, align 8
  %16 = trunc i64 %13 to i32
  store i32 %16, i32* %4, align 8
  br label %9
}

; Function Attrs: nounwind readnone
declare i1 @llvm.expect.i1(i1, i1) #4

; Function Attrs: argmemonly nounwind
declare swiftcc %swift.type* @swift_getTypeByMangledNameInContext(i8*, i64, %swift.type_descriptor*, i8**) #1

; Function Attrs: nounwind
declare zeroext i1 @swift_dynamicCast(%swift.opaque*, %swift.opaque*, %swift.type*, %swift.type*, i64) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

define swiftcc void @"$s5katei3BoxVAASiRszlE3fooyyFZ"() #0 {
entry:
  ret void
}

; Function Attrs: nounwind
define internal %swift.type* @"$s5katei3BoxVMi"(%swift.type_descriptor*, i8**, i8*) #6 {
entry:
  %T1 = alloca %swift.type*, align 8
  %3 = bitcast i8** %1 to %swift.type**
  %T = load %swift.type*, %swift.type** %3, align 8
  store %swift.type* %T, %swift.type** %T1, align 8
  %4 = call %swift.type* @swift_allocateGenericValueMetadata(%swift.type_descriptor* %0, i8** %1, i8* %2, i64 8)
  ret %swift.type* %4
}

; Function Attrs: noinline nounwind readnone
define swiftcc %swift.metadata_response @"$s5katei3BoxVMa"(i64, %swift.type*) #7 {
entry:
  %2 = bitcast %swift.type* %1 to i8*
  %3 = call swiftcc %swift.metadata_response @__swift_instantiateGenericMetadata(i64 %0, i8* %2, i8* undef, i8* undef, %swift.type_descriptor* bitcast (<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn" to %swift.type_descriptor*)) #4
  %4 = extractvalue %swift.metadata_response %3, 0
  %5 = extractvalue %swift.metadata_response %3, 1
  %6 = insertvalue %swift.metadata_response undef, %swift.type* %4, 0
  %7 = insertvalue %swift.metadata_response %6, i64 %5, 1
  ret %swift.metadata_response %7
}

; Function Attrs: nounwind
declare %swift.type* @swift_allocateGenericValueMetadata(%swift.type_descriptor*, i8**, i8*, i64) #5

; Function Attrs: noinline nounwind readnone
define linkonce_odr hidden swiftcc %swift.metadata_response @__swift_instantiateGenericMetadata(i64, i8*, i8*, i8*, %swift.type_descriptor*) #3 {
entry:
  %generic.arguments = alloca [3 x i8*], align 8
  %5 = bitcast [3 x i8*]* %generic.arguments to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %5)
  %6 = getelementptr inbounds [3 x i8*], [3 x i8*]* %generic.arguments, i32 0, i32 0
  store i8* %1, i8** %6, align 8
  %7 = getelementptr inbounds [3 x i8*], [3 x i8*]* %generic.arguments, i32 0, i32 1
  store i8* %2, i8** %7, align 8
  %8 = getelementptr inbounds [3 x i8*], [3 x i8*]* %generic.arguments, i32 0, i32 2
  store i8* %3, i8** %8, align 8
  %9 = bitcast [3 x i8*]* %generic.arguments to i8*
  %10 = call swiftcc %swift.metadata_response @swift_getGenericMetadata(i64 %0, i8* %9, %swift.type_descriptor* %4)
  ret %swift.metadata_response %10
}

; Function Attrs: nounwind readonly
declare swiftcc %swift.metadata_response @swift_getGenericMetadata(i64, i8*, %swift.type_descriptor*) #8

declare extern_weak void @"_swift_FORCE_LOAD_$_swiftCompatibility50"()

declare extern_weak void @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements"()

attributes #0 = { "frame-pointer"="all" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { cold noreturn nounwind }
attributes #3 = { noinline nounwind readnone "frame-pointer"="none" }
attributes #4 = { nounwind readnone }
attributes #5 = { nounwind }
attributes #6 = { nounwind "frame-pointer"="none" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #7 = { noinline nounwind readnone "frame-pointer"="none" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #8 = { nounwind readonly }

!swift.module.flags = !{!0}
!llvm.asan.globals = !{!1, !2, !3, !4, !5, !6, !7}
!llvm.module.flags = !{!8, !9, !10, !11, !12, !13, !14, !15}
!llvm.linker.options = !{!16, !17, !18, !19, !20}

!0 = !{!"standard-library", i1 false}
!1 = !{<{ i8, i64, [4 x i8], i8 }>* @"symbolic _________ySiG 5katei3BoxV", null, null, i1 false, i1 true}
!2 = !{<{ i32, i64, i64 }>* @"$s5kateiMXM", null, null, i1 false, i1 true}
!3 = !{<{ i32, i64, i64, i64, i64, i32, i32, i64, i64, i16, i16, i16, i16, i8, i8, i8, i8 }>* @"$s5katei3BoxVMn", null, null, i1 false, i1 true}
!4 = !{<{ i8, i64, i8 }>* @"symbolic _________ 5katei3BoxV", null, null, i1 false, i1 true}
!5 = !{{ i64, i32, i16, i16, i32 }* @"$s5katei3BoxVMF", null, null, i1 false, i1 true}
!6 = !{[1 x %swift.type_metadata_record]* @"\01l_type_metadata_table", null, null, i1 false, i1 true}
!7 = !{[9 x i8*]* @llvm.used, null, null, i1 false, i1 true}
!8 = !{i32 1, !"Objective-C Version", i32 2}
!9 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!10 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!11 = !{i32 4, !"Objective-C Garbage Collection", i32 83953408}
!12 = !{i32 1, !"Objective-C Class Properties", i32 64}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 7, !"PIC Level", i32 2}
!15 = !{i32 1, !"Swift Version", i32 7}
!16 = !{!"-lswiftSwiftOnoneSupport"}
!17 = !{!"-lswiftCore"}
!18 = !{!"-lobjc"}
!19 = !{!"-lswiftCompatibility50"}
!20 = !{!"-lswiftCompatibilityDynamicReplacements"}
