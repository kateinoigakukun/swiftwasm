; ModuleID = './katei.ll'
source_filename = "./katei.ll"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%swift.full_type = type { i8**, %swift.type }
%swift.type = type { i64 }
%swift.protocol_conformance_descriptor = type { i64, i64, i64, i32 }
%swift.protocol_requirement = type { i32, i64 }
%swift.protocolref = type { i64 }
%swift.type_metadata_record = type { i64 }
%Any = type { [24 x i8], %swift.type* }
%T4main1AV = type opaque
%__opaque_existential_type_0 = type { [24 x i8], %swift.type* }
%T4main1PP = type { [24 x i8], %swift.type*, i8** }
%__opaque_existential_type_1 = type { [24 x i8], %swift.type*, i8** }
%swift.opaque = type opaque
%swift.vwtable = type { i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i64, i64, i32, i32 }
%swift.refcounted = type { %swift.type*, i64 }
%swift.type_descriptor = type opaque
%swift.metadata_response = type { %swift.type*, i64 }

@"$sypN" = external global %swift.full_type
@"symbolic ______p 4main1PP" = linkonce_odr hidden constant <{ i8, i64, [2 x i8], i8 }> <{ i8 1, i64 ptrtoint (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp" to i64), [2 x i8] c"_p", i8 0 }>, section "__DATA,__swift5_typeref, regular, no_dead_strip", align 2
@"$s4main1P_pMD" = linkonce_odr hidden global { i64, i32 } { i64 ptrtoint (<{ i8, i64, [2 x i8], i8 }>* @"symbolic ______p 4main1PP" to i64), i32 -7 }, align 8
@"$s4main1AVAA1PAAMc" = constant %swift.protocol_conformance_descriptor { i64 ptrtoint (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp" to i64), i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn" to i64), i64 ptrtoint ([2 x i8*]* @"$s4main1AVAA1PAAWP" to i64), i32 0 }, section "__DATA,__const", align 4
@"$s4main1AVAA1PAAWP" = constant [2 x i8*] [i8* bitcast (%swift.protocol_conformance_descriptor* @"$s4main1AVAA1PAAMc" to i8*), i8* bitcast (void (%T4main1AV*, %swift.type*, i8**)* @"$s4main1AVAA1PA2aDP3fooyyFTW" to i8*)], align 8
@"symbolic $s4main1PP" = linkonce_odr hidden constant <{ [10 x i8], i8 }> <{ [10 x i8] c"$s4main1PP", i8 0 }>, section "__DATA,__swift5_typeref, regular, no_dead_strip", align 2
@"$s4main1P_pMF" = internal constant { i64, i32, i16, i16, i32 } { i64 ptrtoint (<{ [10 x i8], i8 }>* @"symbolic $s4main1PP" to i64), i32 0, i16 4, i16 12, i32 0 }, section "__DATA,__swift5_fieldmd, regular, no_dead_strip", align 4
@0 = private constant [5 x i8] c"main\00"
@"$s4mainMXM" = linkonce_odr hidden constant <{ i32, i32, i64 }> <{ i32 0, i32 0, i64 ptrtoint ([5 x i8]* @0 to i64) }>, section "__DATA,__const", align 4
@1 = private constant [2 x i8] c"P\00"
@"$s4main1PMp" = constant <{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }> <{ i32 65603, i64 ptrtoint (<{ i32, i32, i64 }>* @"$s4mainMXM" to i64), i64 ptrtoint ([2 x i8]* @1 to i64), i32 0, i32 1, i64 0, %swift.protocol_requirement { i32 17, i64 0 } }>, section "__DATA,__const", align 4
@"$sytWV" = external global i8*, align 8
@2 = private constant [2 x i8] c"A\00"
@"$s4main1AVMn" = constant <{ i32, i64, i64, i64, i64, i32, i32 }> <{ i32 81, i64 ptrtoint (<{ i32, i32, i64 }>* @"$s4mainMXM" to i64), i64 ptrtoint ([2 x i8]* @2 to i64), i64 ptrtoint (%swift.metadata_response (i64)* @"$s4main1AVMa" to i64), i64 ptrtoint ({ i64, i32, i16, i16, i32 }* @"$s4main1AVMF" to i64), i32 0, i32 2 }>, section "__DATA,__const", align 4
@"$s4main1AVMf" = internal constant <{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }> <{ i8** @"$sytWV", i64 512, <{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn" }>, align 8
@"symbolic _____ 4main1AV" = linkonce_odr hidden constant <{ i8, i64, i8 }> <{ i8 1, i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn" to i64), i8 0 }>, section "__DATA,__swift5_typeref, regular, no_dead_strip", align 2
@"$s4main1AVMF" = internal constant { i64, i32, i16, i16, i32 } { i64 ptrtoint (<{ i8, i64, i8 }>* @"symbolic _____ 4main1AV" to i64), i32 0, i16 0, i16 12, i32 0 }, section "__DATA,__swift5_fieldmd, regular, no_dead_strip", align 4
@"_swift_FORCE_LOAD_$_swiftCompatibility50_$_main" = weak_odr hidden constant void ()* @"_swift_FORCE_LOAD_$_swiftCompatibility50"
@"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements_$_main" = weak_odr hidden constant void ()* @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements"
@"\01l_protocols" = private constant [1 x %swift.protocolref] [%swift.protocolref { i64 ptrtoint (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp" to i64) }], section "__DATA, __swift5_protos, regular, no_dead_strip", align 4
@"\01l_protocol_conformances" = private constant [1 x i64] [i64 ptrtoint (%swift.protocol_conformance_descriptor* @"$s4main1AVAA1PAAMc" to i64)], section "__DATA, __swift5_proto, regular, no_dead_strip", align 4
@"\01l_type_metadata_table" = private constant [1 x %swift.type_metadata_record] [%swift.type_metadata_record { i64 ptrtoint (<{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn" to i64) }], section "__DATA, __swift5_types, regular, no_dead_strip", align 4
@__swift_reflection_version = linkonce_odr hidden constant i16 3
@llvm.used = appending global [19 x i8*] [i8* bitcast (void ()* @"$s4main1AV3fooyyF" to i8*), i8* bitcast (void (%T4main1PP*)* @"$s4main7callFooyyAA1P_pF" to i8*), i8* bitcast (void (%Any*)* @"$s4main14callFooFromAnyyyypF" to i8*), i8* bitcast ([2 x i8*]* @"$s4main1AVAA1PAAWP" to i8*), i8* bitcast ({ i64, i32, i16, i16, i32 }* @"$s4main1P_pMF" to i8*), i8* bitcast (%swift.protocol_requirement* @"$s4main1PTL" to i8*), i8* bitcast (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp" to i8*), i8* bitcast (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp" to i8*), i8* bitcast (<{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn" to i8*), i8* bitcast (%swift.metadata_response (i64)* @"$s4main1AVMa" to i8*), i8* bitcast (%swift.type* @"$s4main1AVN" to i8*), i8* bitcast ({ i64, i32, i16, i16, i32 }* @"$s4main1AVMF" to i8*), i8* bitcast (void ()** @"_swift_FORCE_LOAD_$_swiftCompatibility50_$_main" to i8*), i8* bitcast (void ()** @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements_$_main" to i8*), i8* bitcast ([1 x %swift.protocolref]* @"\01l_protocols" to i8*), i8* bitcast (%swift.protocol_conformance_descriptor* @"$s4main1AVAA1PAAMc" to i8*), i8* bitcast ([1 x i64]* @"\01l_protocol_conformances" to i8*), i8* bitcast ([1 x %swift.type_metadata_record]* @"\01l_type_metadata_table" to i8*), i8* bitcast (i16* @__swift_reflection_version to i8*)], section "llvm.metadata", align 8

@"$s4main1PTL" = alias %swift.protocol_requirement, getelementptr (%swift.protocol_requirement, %swift.protocol_requirement* getelementptr inbounds (<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>, <{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp", i32 0, i32 6), i32 -1)
@"$s4main1AVN" = alias %swift.type, bitcast (i64* getelementptr inbounds (<{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>, <{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>* @"$s4main1AVMf", i32 0, i32 1) to %swift.type*)

define i32 @main(i32, i8**) #0 {
entry:
  %2 = alloca %Any, align 8
  %3 = bitcast i8** %1 to i8*
  %4 = bitcast %Any* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* %4)
  call swiftcc void @"$s4main1AVACycfC"()
  %5 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 1
  store %swift.type* bitcast (i64* getelementptr inbounds (<{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>, <{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>* @"$s4main1AVMf", i32 0, i32 1) to %swift.type*), %swift.type** %5, align 8
  %6 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 0
  %7 = getelementptr inbounds %Any, %Any* %2, i32 0, i32 0
  %8 = bitcast [24 x i8]* %7 to %T4main1AV*
  call swiftcc void @"$s4main14callFooFromAnyyyypF"(%Any* noalias nocapture dereferenceable(32) %2)
  %9 = bitcast %Any* %2 to %__opaque_existential_type_0*
  call void @__swift_destroy_boxed_opaque_existential_0(%__opaque_existential_type_0* %9) #4
  %10 = bitcast %Any* %2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 32, i8* %10)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

define swiftcc void @"$s4main1AV3fooyyF"() #0 {
entry:
  ret void
}

define hidden swiftcc void @"$s4main1AVACycfC"() #0 {
entry:
  ret void
}

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #2

define linkonce_odr hidden swiftcc void @"$s4main1AVAA1PA2aDP3fooyyFTW"(%T4main1AV* noalias nocapture swiftself, %swift.type* %Self, i8** %SelfWitnessTable) #0 {
entry:
  call swiftcc void @"$s4main1AV3fooyyF"() #8
  ret void
}

define swiftcc void @"$s4main7callFooyyAA1P_pF"(%T4main1PP* noalias nocapture dereferenceable(40)) #0 {
entry:
  %1 = getelementptr inbounds %T4main1PP, %T4main1PP* %0, i32 0, i32 1
  %2 = load %swift.type*, %swift.type** %1, align 8
  %3 = getelementptr inbounds %T4main1PP, %T4main1PP* %0, i32 0, i32 2
  %4 = load i8**, i8*** %3, align 8
  %5 = bitcast %T4main1PP* %0 to %__opaque_existential_type_1*
  %6 = call %swift.opaque* @__swift_project_boxed_opaque_existential_1(%__opaque_existential_type_1* %5, %swift.type* %2) #4
  %7 = getelementptr inbounds i8*, i8** %4, i32 1
  %8 = load i8*, i8** %7, align 8, !invariant.load !27
  %9 = bitcast i8* %8 to void (%swift.opaque*, %swift.type*, i8**)*
  call swiftcc void %9(%swift.opaque* noalias nocapture swiftself %6, %swift.type* %2, i8** %4)
  ret void
}

define swiftcc void @"$s4main14callFooFromAnyyyypF"(%Any* noalias nocapture dereferenceable(32)) #0 {
entry:
  %1 = alloca %Any, align 8
  %2 = alloca %T4main1PP, align 8
  %3 = bitcast %Any* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* %3)
  %4 = call %Any* @"$sypWOc"(%Any* %0, %Any* %1)
  %5 = bitcast %T4main1PP* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %5)
  %6 = bitcast %T4main1PP* %2 to %swift.opaque*
  %7 = bitcast %Any* %1 to %swift.opaque*
  %8 = call %swift.type* @__swift_instantiateConcreteTypeFromMangledName({ i64, i32 }* @"$s4main1P_pMD") #6
  %9 = call i1 @swift_dynamicCast(%swift.opaque* %6, %swift.opaque* %7, %swift.type* getelementptr inbounds (%swift.full_type, %swift.full_type* @"$sypN", i32 0, i32 1), %swift.type* %8, i64 7) #4
  call swiftcc void @"$s4main7callFooyyAA1P_pF"(%T4main1PP* noalias nocapture dereferenceable(40) %2)
  %10 = bitcast %T4main1PP* %2 to %__opaque_existential_type_1*
  call void @__swift_destroy_boxed_opaque_existential_1(%__opaque_existential_type_1* %10) #4
  %11 = bitcast %T4main1PP* %2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %11)
  %12 = bitcast %Any* %1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 32, i8* %12)
  ret void
}

; Function Attrs: noinline nounwind
define linkonce_odr hidden void @__swift_destroy_boxed_opaque_existential_0(%__opaque_existential_type_0*) #3 {
entry:
  %1 = getelementptr inbounds %__opaque_existential_type_0, %__opaque_existential_type_0* %0, i32 0, i32 1
  %2 = load %swift.type*, %swift.type** %1, align 8
  %3 = getelementptr inbounds %__opaque_existential_type_0, %__opaque_existential_type_0* %0, i32 0, i32 0
  %4 = bitcast %swift.type* %2 to i8***
  %5 = getelementptr inbounds i8**, i8*** %4, i64 -1
  %.valueWitnesses = load i8**, i8*** %5, align 8, !invariant.load !27, !dereferenceable !28
  %6 = bitcast i8** %.valueWitnesses to %swift.vwtable*
  %7 = getelementptr inbounds %swift.vwtable, %swift.vwtable* %6, i32 0, i32 10
  %flags = load i32, i32* %7, align 8, !invariant.load !27
  %8 = and i32 %flags, 131072
  %flags.isInline = icmp eq i32 %8, 0
  br i1 %flags.isInline, label %inline, label %outline

inline:                                           ; preds = %entry
  %9 = bitcast [24 x i8]* %3 to %swift.opaque*
  %10 = bitcast %swift.type* %2 to i8***
  %11 = getelementptr inbounds i8**, i8*** %10, i64 -1
  %.valueWitnesses1 = load i8**, i8*** %11, align 8, !invariant.load !27, !dereferenceable !28
  %12 = getelementptr inbounds i8*, i8** %.valueWitnesses1, i32 1
  %13 = load i8*, i8** %12, align 8, !invariant.load !27
  %destroy = bitcast i8* %13 to void (%swift.opaque*, %swift.type*)*
  call void %destroy(%swift.opaque* noalias %9, %swift.type* %2) #4
  ret void

outline:                                          ; preds = %entry
  %14 = bitcast [24 x i8]* %3 to %swift.refcounted**
  %15 = load %swift.refcounted*, %swift.refcounted** %14, align 8
  call void @swift_release(%swift.refcounted* %15) #4
  ret void
}

; Function Attrs: nounwind
declare void @swift_release(%swift.refcounted*) #4

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: noinline nounwind
define linkonce_odr hidden %swift.opaque* @__swift_project_boxed_opaque_existential_1(%__opaque_existential_type_1*, %swift.type*) #3 {
entry:
  %2 = bitcast %swift.type* %1 to i8***
  %3 = getelementptr inbounds i8**, i8*** %2, i64 -1
  %.valueWitnesses = load i8**, i8*** %3, align 8, !invariant.load !27, !dereferenceable !28
  %4 = bitcast i8** %.valueWitnesses to %swift.vwtable*
  %5 = getelementptr inbounds %swift.vwtable, %swift.vwtable* %4, i32 0, i32 10
  %flags = load i32, i32* %5, align 8, !invariant.load !27
  %6 = and i32 %flags, 131072
  %flags.isInline = icmp eq i32 %6, 0
  %7 = bitcast %__opaque_existential_type_1* %0 to %swift.opaque*
  br i1 %flags.isInline, label %done, label %boxed

done:                                             ; preds = %entry
  ret %swift.opaque* %7

boxed:                                            ; preds = %entry
  %8 = bitcast %__opaque_existential_type_1* %0 to %swift.refcounted**
  %9 = load %swift.refcounted*, %swift.refcounted** %8, align 8
  %10 = zext i32 %flags to i64
  %flags.alignmentMask = and i64 %10, 255
  %11 = add i64 16, %flags.alignmentMask
  %12 = xor i64 %flags.alignmentMask, -1
  %13 = and i64 %11, %12
  %14 = bitcast %swift.refcounted* %9 to i8*
  %15 = getelementptr inbounds i8, i8* %14, i64 %13
  %16 = bitcast i8* %15 to %swift.opaque*
  ret %swift.opaque* %16
}

; Function Attrs: noinline nounwind
define linkonce_odr hidden %Any* @"$sypWOc"(%Any*, %Any*) #3 {
entry:
  %2 = getelementptr inbounds %Any, %Any* %0, i32 0, i32 1
  %3 = load %swift.type*, %swift.type** %2, align 8
  %4 = getelementptr inbounds %Any, %Any* %1, i32 0, i32 1
  store %swift.type* %3, %swift.type** %4, align 8
  %5 = getelementptr inbounds %Any, %Any* %0, i32 0, i32 0
  %6 = getelementptr inbounds %Any, %Any* %1, i32 0, i32 0
  %7 = bitcast %swift.type* %3 to i8***
  %8 = getelementptr inbounds i8**, i8*** %7, i64 -1
  %.valueWitnesses = load i8**, i8*** %8, align 8, !invariant.load !27, !dereferenceable !28
  %9 = load i8*, i8** %.valueWitnesses, align 8, !invariant.load !27
  %initializeBufferWithCopyOfBuffer = bitcast i8* %9 to %swift.opaque* ([24 x i8]*, [24 x i8]*, %swift.type*)*
  %10 = call %swift.opaque* %initializeBufferWithCopyOfBuffer([24 x i8]* noalias %6, [24 x i8]* noalias %5, %swift.type* %3) #4
  ret %Any* %1
}

; Function Attrs: noinline nounwind readnone
define linkonce_odr hidden %swift.type* @__swift_instantiateConcreteTypeFromMangledName({ i64, i32 }*) #5 {
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
  %14 = call swiftcc %swift.type* @swift_getTypeByMangledNameInContext(i8* %3, i64 %13, %swift.type_descriptor* null, i8** null) #6
  %15 = ptrtoint %swift.type* %14 to i64
  store atomic i64 %15, i64* %1 monotonic, align 8
  %16 = trunc i64 %13 to i32
  store i32 %16, i32* %4, align 8
  br label %9
}

; Function Attrs: nounwind readnone
declare i1 @llvm.expect.i1(i1, i1) #6

; Function Attrs: argmemonly nounwind
declare swiftcc %swift.type* @swift_getTypeByMangledNameInContext(i8*, i64, %swift.type_descriptor*, i8**) #1

; Function Attrs: nounwind
declare zeroext i1 @swift_dynamicCast(%swift.opaque*, %swift.opaque*, %swift.type*, %swift.type*, i64) #4

; Function Attrs: noinline nounwind
define linkonce_odr hidden void @__swift_destroy_boxed_opaque_existential_1(%__opaque_existential_type_1*) #3 {
entry:
  %1 = getelementptr inbounds %__opaque_existential_type_1, %__opaque_existential_type_1* %0, i32 0, i32 1
  %2 = load %swift.type*, %swift.type** %1, align 8
  %3 = getelementptr inbounds %__opaque_existential_type_1, %__opaque_existential_type_1* %0, i32 0, i32 0
  %4 = bitcast %swift.type* %2 to i8***
  %5 = getelementptr inbounds i8**, i8*** %4, i64 -1
  %.valueWitnesses = load i8**, i8*** %5, align 8, !invariant.load !27, !dereferenceable !28
  %6 = bitcast i8** %.valueWitnesses to %swift.vwtable*
  %7 = getelementptr inbounds %swift.vwtable, %swift.vwtable* %6, i32 0, i32 10
  %flags = load i32, i32* %7, align 8, !invariant.load !27
  %8 = and i32 %flags, 131072
  %flags.isInline = icmp eq i32 %8, 0
  br i1 %flags.isInline, label %inline, label %outline

inline:                                           ; preds = %entry
  %9 = bitcast [24 x i8]* %3 to %swift.opaque*
  %10 = bitcast %swift.type* %2 to i8***
  %11 = getelementptr inbounds i8**, i8*** %10, i64 -1
  %.valueWitnesses1 = load i8**, i8*** %11, align 8, !invariant.load !27, !dereferenceable !28
  %12 = getelementptr inbounds i8*, i8** %.valueWitnesses1, i32 1
  %13 = load i8*, i8** %12, align 8, !invariant.load !27
  %destroy = bitcast i8* %13 to void (%swift.opaque*, %swift.type*)*
  call void %destroy(%swift.opaque* noalias %9, %swift.type* %2) #4
  ret void

outline:                                          ; preds = %entry
  %14 = bitcast [24 x i8]* %3 to %swift.refcounted**
  %15 = load %swift.refcounted*, %swift.refcounted** %14, align 8
  call void @swift_release(%swift.refcounted* %15) #4
  ret void
}

; Function Attrs: noinline nounwind readnone
define swiftcc %swift.metadata_response @"$s4main1AVMa"(i64) #7 {
entry:
  ret %swift.metadata_response { %swift.type* bitcast (i64* getelementptr inbounds (<{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>, <{ i8**, i64, <{ i32, i64, i64, i64, i64, i32, i32 }>* }>* @"$s4main1AVMf", i32 0, i32 1) to %swift.type*), i64 0 }
}

declare extern_weak void @"_swift_FORCE_LOAD_$_swiftCompatibility50"()

declare extern_weak void @"_swift_FORCE_LOAD_$_swiftCompatibilityDynamicReplacements"()

attributes #0 = { "frame-pointer"="all" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { cold noreturn nounwind }
attributes #3 = { noinline nounwind }
attributes #4 = { nounwind }
attributes #5 = { noinline nounwind readnone "frame-pointer"="none" }
attributes #6 = { nounwind readnone }
attributes #7 = { noinline nounwind readnone "frame-pointer"="none" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #8 = { noinline }

!swift.module.flags = !{!0}
!llvm.asan.globals = !{!1, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13}
!llvm.module.flags = !{!14, !15, !16, !17, !18, !19, !20, !21}
!llvm.linker.options = !{!22, !23, !24, !25, !26}

!0 = !{!"standard-library", i1 false}
!1 = !{<{ i8, i64, [2 x i8], i8 }>* @"symbolic ______p 4main1PP", null, null, i1 false, i1 true}
!2 = !{<{ [10 x i8], i8 }>* @"symbolic $s4main1PP", null, null, i1 false, i1 true}
!3 = !{{ i64, i32, i16, i16, i32 }* @"$s4main1P_pMF", null, null, i1 false, i1 true}
!4 = !{<{ i32, i32, i64 }>* @"$s4mainMXM", null, null, i1 false, i1 true}
!5 = !{<{ i32, i64, i64, i32, i32, i64, %swift.protocol_requirement }>* @"$s4main1PMp", null, null, i1 false, i1 true}
!6 = !{<{ i32, i64, i64, i64, i64, i32, i32 }>* @"$s4main1AVMn", null, null, i1 false, i1 true}
!7 = !{<{ i8, i64, i8 }>* @"symbolic _____ 4main1AV", null, null, i1 false, i1 true}
!8 = !{{ i64, i32, i16, i16, i32 }* @"$s4main1AVMF", null, null, i1 false, i1 true}
!9 = !{[1 x %swift.protocolref]* @"\01l_protocols", null, null, i1 false, i1 true}
!10 = !{%swift.protocol_conformance_descriptor* @"$s4main1AVAA1PAAMc", null, null, i1 false, i1 true}
!11 = !{[1 x i64]* @"\01l_protocol_conformances", null, null, i1 false, i1 true}
!12 = !{[1 x %swift.type_metadata_record]* @"\01l_type_metadata_table", null, null, i1 false, i1 true}
!13 = !{[19 x i8*]* @llvm.used, null, null, i1 false, i1 true}
!14 = !{i32 1, !"Objective-C Version", i32 2}
!15 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!16 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!17 = !{i32 4, !"Objective-C Garbage Collection", i32 83953408}
!18 = !{i32 1, !"Objective-C Class Properties", i32 64}
!19 = !{i32 1, !"wchar_size", i32 4}
!20 = !{i32 7, !"PIC Level", i32 2}
!21 = !{i32 1, !"Swift Version", i32 7}
!22 = !{!"-lswiftSwiftOnoneSupport"}
!23 = !{!"-lswiftCore"}
!24 = !{!"-lobjc"}
!25 = !{!"-lswiftCompatibility50"}
!26 = !{!"-lswiftCompatibilityDynamicReplacements"}
!27 = !{}
!28 = !{i64 96}
