// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <string>
#include <vector>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using InlineTest = PassTest<::testing::Test>;

TEST_F(InlineTest, Simple) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     return bar.x + bar.y;
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %gl_FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 140",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %color \"color\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %gl_FragColor \"gl_FragColor\"",
       "%void = OpTypeVoid",
         "%10 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%14 = OpTypeFunction %float %_ptr_Function_v4float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
"%_ptr_Function_float = OpTypePointer Function %float",
     "%uint_1 = OpConstant %uint 1",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
"%gl_FragColor = OpVariable %_ptr_Output_v4float Output",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %float None %14",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%26 = OpLabel",
         "%27 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%28 = OpLoad %float %27",
         "%29 = OpAccessChain %_ptr_Function_float %bar %uint_1",
         "%30 = OpLoad %float %29",
         "%31 = OpFAdd %float %28 %30",
               "OpReturnValue %31",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %10",
         "%21 = OpLabel",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
         "%22 = OpLoad %v4float %BaseColor",
               "OpStore %param %22",
         "%23 = OpFunctionCall %float %foo_vf4_ %param",
         "%24 = OpCompositeConstruct %v4float %23 %23 %23 %23",
               "OpStore %color %24",
         "%25 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %25",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %10",
         "%21 = OpLabel",
         "%32 = OpVariable %_ptr_Function_float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
         "%22 = OpLoad %v4float %BaseColor",
               "OpStore %param %22",
         "%33 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%34 = OpLoad %float %33",
         "%35 = OpAccessChain %_ptr_Function_float %param %uint_1",
         "%36 = OpLoad %float %35",
         "%37 = OpFAdd %float %34 %36",
               "OpStore %32 %37",
         "%23 = OpLoad %float %32",
         "%24 = OpCompositeConstruct %v4float %23 %23 %23 %23",
               "OpStore %color %24",
         "%25 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %25",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, Nested) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo2(float f, float f2)
  // {
  //     return f * f2;
  // }
  //
  // float foo(vec4 bar)
  // {
  //     return foo2(bar.x + bar.y, bar.z);
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %gl_FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 140",
               "OpName %main \"main\"",
               "OpName %foo2_f1_f1_ \"foo2(f1;f1;\"",
               "OpName %f \"f\"",
               "OpName %f2 \"f2\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %param \"param\"",
               "OpName %param_0 \"param\"",
               "OpName %color \"color\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param_1 \"param\"",
               "OpName %gl_FragColor \"gl_FragColor\"",
       "%void = OpTypeVoid",
         "%15 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
"%_ptr_Function_float = OpTypePointer Function %float",
         "%18 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%21 = OpTypeFunction %float %_ptr_Function_v4float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
     "%uint_1 = OpConstant %uint 1",
     "%uint_2 = OpConstant %uint 2",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
"%gl_FragColor = OpVariable %_ptr_Output_v4float Output",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
"%foo2_f1_f1_ = OpFunction %float None %18",
          "%f = OpFunctionParameter %_ptr_Function_float",
         "%f2 = OpFunctionParameter %_ptr_Function_float",
         "%33 = OpLabel",
         "%34 = OpLoad %float %f",
         "%35 = OpLoad %float %f2",
         "%36 = OpFMul %float %34 %35",
               "OpReturnValue %36",
               "OpFunctionEnd",
   "%foo_vf4_ = OpFunction %float None %21",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%37 = OpLabel",
      "%param = OpVariable %_ptr_Function_float Function",
    "%param_0 = OpVariable %_ptr_Function_float Function",
         "%38 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%39 = OpLoad %float %38",
         "%40 = OpAccessChain %_ptr_Function_float %bar %uint_1",
         "%41 = OpLoad %float %40",
         "%42 = OpFAdd %float %39 %41",
               "OpStore %param %42",
         "%43 = OpAccessChain %_ptr_Function_float %bar %uint_2",
         "%44 = OpLoad %float %43",
               "OpStore %param_0 %44",
         "%45 = OpFunctionCall %float %foo2_f1_f1_ %param %param_0",
               "OpReturnValue %45",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %15",
         "%28 = OpLabel",
      "%color = OpVariable %_ptr_Function_v4float Function",
    "%param_1 = OpVariable %_ptr_Function_v4float Function",
         "%29 = OpLoad %v4float %BaseColor",
               "OpStore %param_1 %29",
         "%30 = OpFunctionCall %float %foo_vf4_ %param_1",
         "%31 = OpCompositeConstruct %v4float %30 %30 %30 %30",
               "OpStore %color %31",
         "%32 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %32",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %15",
         "%28 = OpLabel",
         "%57 = OpVariable %_ptr_Function_float Function",
         "%46 = OpVariable %_ptr_Function_float Function",
         "%47 = OpVariable %_ptr_Function_float Function",
         "%48 = OpVariable %_ptr_Function_float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
    "%param_1 = OpVariable %_ptr_Function_v4float Function",
         "%29 = OpLoad %v4float %BaseColor",
               "OpStore %param_1 %29",
         "%49 = OpAccessChain %_ptr_Function_float %param_1 %uint_0",
         "%50 = OpLoad %float %49",
         "%51 = OpAccessChain %_ptr_Function_float %param_1 %uint_1",
         "%52 = OpLoad %float %51",
         "%53 = OpFAdd %float %50 %52",
               "OpStore %46 %53",
         "%54 = OpAccessChain %_ptr_Function_float %param_1 %uint_2",
         "%55 = OpLoad %float %54",
               "OpStore %47 %55",
         "%58 = OpLoad %float %46",
         "%59 = OpLoad %float %47",
         "%60 = OpFMul %float %58 %59",
               "OpStore %57 %60",
         "%56 = OpLoad %float %57",
               "OpStore %48 %56",
         "%30 = OpLoad %float %48",
         "%31 = OpCompositeConstruct %v4float %30 %30 %30 %30",
               "OpStore %color %31",
         "%32 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %32",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, InOutParameter) {
  // #version 400
  //
  // in vec4 Basecolor;
  //
  // void foo(inout vec4 bar)
  // {
  //     bar.z = bar.x + bar.y;
  // }
  //
  // void main()
  // {
  //     vec4 b = Basecolor;
  //     foo(b);
  //     vec4 color = vec4(b.z);
  //     gl_FragColor = color;
  // }
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %Basecolor %gl_FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 400",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %b \"b\"",
               "OpName %Basecolor \"Basecolor\"",
               "OpName %param \"param\"",
               "OpName %color \"color\"",
               "OpName %gl_FragColor \"gl_FragColor\"",
       "%void = OpTypeVoid",
         "%11 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%15 = OpTypeFunction %void %_ptr_Function_v4float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
"%_ptr_Function_float = OpTypePointer Function %float",
     "%uint_1 = OpConstant %uint 1",
     "%uint_2 = OpConstant %uint 2",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%Basecolor = OpVariable %_ptr_Input_v4float Input",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
"%gl_FragColor = OpVariable %_ptr_Output_v4float Output",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %void None %15",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%32 = OpLabel",
         "%33 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%34 = OpLoad %float %33",
         "%35 = OpAccessChain %_ptr_Function_float %bar %uint_1",
         "%36 = OpLoad %float %35",
         "%37 = OpFAdd %float %34 %36",
         "%38 = OpAccessChain %_ptr_Function_float %bar %uint_2",
               "OpStore %38 %37",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %11",
         "%23 = OpLabel",
          "%b = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
         "%24 = OpLoad %v4float %Basecolor",
               "OpStore %b %24",
         "%25 = OpLoad %v4float %b",
               "OpStore %param %25",
         "%26 = OpFunctionCall %void %foo_vf4_ %param",
         "%27 = OpLoad %v4float %param",
               "OpStore %b %27",
         "%28 = OpAccessChain %_ptr_Function_float %b %uint_2",
         "%29 = OpLoad %float %28",
         "%30 = OpCompositeConstruct %v4float %29 %29 %29 %29",
               "OpStore %color %30",
         "%31 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %31",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %11",
         "%23 = OpLabel",
          "%b = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
         "%24 = OpLoad %v4float %Basecolor",
               "OpStore %b %24",
         "%25 = OpLoad %v4float %b",
               "OpStore %param %25",
         "%39 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%40 = OpLoad %float %39",
         "%41 = OpAccessChain %_ptr_Function_float %param %uint_1",
         "%42 = OpLoad %float %41",
         "%43 = OpFAdd %float %40 %42",
         "%44 = OpAccessChain %_ptr_Function_float %param %uint_2",
               "OpStore %44 %43",
         "%27 = OpLoad %v4float %param",
               "OpStore %b %27",
         "%28 = OpAccessChain %_ptr_Function_float %b %uint_2",
         "%29 = OpLoad %float %28",
         "%30 = OpCompositeConstruct %v4float %29 %29 %29 %29",
               "OpStore %color %30",
         "%31 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %31",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, BranchInCallee) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     float r = bar.x;
  //     if (r < 0.0)
  //         r = -r;
  //     return r;
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //
  //     gl_FragColor = color;
  // }
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %gl_FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 140",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %r \"r\"",
               "OpName %color \"color\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %gl_FragColor \"gl_FragColor\"",
       "%void = OpTypeVoid",
         "%11 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%15 = OpTypeFunction %float %_ptr_Function_v4float",
"%_ptr_Function_float = OpTypePointer Function %float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
    "%float_0 = OpConstant %float 0",
       "%bool = OpTypeBool",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
"%gl_FragColor = OpVariable %_ptr_Output_v4float Output",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %float None %15",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%28 = OpLabel",
          "%r = OpVariable %_ptr_Function_float Function",
         "%29 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%30 = OpLoad %float %29",
               "OpStore %r %30",
         "%31 = OpLoad %float %r",
         "%32 = OpFOrdLessThan %bool %31 %float_0",
               "OpSelectionMerge %33 None",
               "OpBranchConditional %32 %34 %33",
         "%34 = OpLabel",
         "%35 = OpLoad %float %r",
         "%36 = OpFNegate %float %35",
               "OpStore %r %36",
               "OpBranch %33",
         "%33 = OpLabel",
         "%37 = OpLoad %float %r",
               "OpReturnValue %37",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %11",
         "%23 = OpLabel",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
         "%24 = OpLoad %v4float %BaseColor",
               "OpStore %param %24",
         "%25 = OpFunctionCall %float %foo_vf4_ %param",
         "%26 = OpCompositeConstruct %v4float %25 %25 %25 %25",
               "OpStore %color %26",
         "%27 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %27",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %11",
         "%23 = OpLabel",
         "%38 = OpVariable %_ptr_Function_float Function",
         "%39 = OpVariable %_ptr_Function_float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
         "%24 = OpLoad %v4float %BaseColor",
               "OpStore %param %24",
         "%40 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%41 = OpLoad %float %40",
               "OpStore %38 %41",
         "%42 = OpLoad %float %38",
         "%43 = OpFOrdLessThan %bool %42 %float_0",
               "OpSelectionMerge %44 None",
               "OpBranchConditional %43 %45 %44",
         "%45 = OpLabel",
         "%46 = OpLoad %float %38",
         "%47 = OpFNegate %float %46",
               "OpStore %38 %47",
               "OpBranch %44",
         "%44 = OpLabel",
         "%48 = OpLoad %float %38",
               "OpStore %39 %48",
         "%25 = OpLoad %float %39",
         "%26 = OpCompositeConstruct %v4float %25 %25 %25 %25",
               "OpStore %color %26",
         "%27 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %27",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, PhiAfterCall) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(float bar)
  // {
  //     float r = bar;
  //     if (r < 0.0)
  //         r = -r;
  //     return r;
  // }
  //
  // void main()
  // {
  //     vec4 color = BaseColor;
  //     if (foo(color.x) > 2.0 && foo(color.y) > 2.0)
  //         color = vec4(0.0);
  //     gl_FragColor = color;
  // }
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %gl_FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 140",
               "OpName %main \"main\"",
               "OpName %foo_f1_ \"foo(f1;\"",
               "OpName %bar \"bar\"",
               "OpName %r \"r\"",
               "OpName %color \"color\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %param_0 \"param\"",
               "OpName %gl_FragColor \"gl_FragColor\"",
       "%void = OpTypeVoid",
         "%12 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
"%_ptr_Function_float = OpTypePointer Function %float",
         "%15 = OpTypeFunction %float %_ptr_Function_float",
    "%float_0 = OpConstant %float 0",
       "%bool = OpTypeBool",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
    "%float_2 = OpConstant %float 2",
     "%uint_1 = OpConstant %uint 1",
         "%25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
"%gl_FragColor = OpVariable %_ptr_Output_v4float Output",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
    "%foo_f1_ = OpFunction %float None %15",
        "%bar = OpFunctionParameter %_ptr_Function_float",
         "%43 = OpLabel",
          "%r = OpVariable %_ptr_Function_float Function",
         "%44 = OpLoad %float %bar",
               "OpStore %r %44",
         "%45 = OpLoad %float %r",
         "%46 = OpFOrdLessThan %bool %45 %float_0",
               "OpSelectionMerge %47 None",
               "OpBranchConditional %46 %48 %47",
         "%48 = OpLabel",
         "%49 = OpLoad %float %r",
         "%50 = OpFNegate %float %49",
               "OpStore %r %50",
               "OpBranch %47",
         "%47 = OpLabel",
         "%51 = OpLoad %float %r",
               "OpReturnValue %51",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %12",
         "%27 = OpLabel",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_float Function",
    "%param_0 = OpVariable %_ptr_Function_float Function",
         "%28 = OpLoad %v4float %BaseColor",
               "OpStore %color %28",
         "%29 = OpAccessChain %_ptr_Function_float %color %uint_0",
         "%30 = OpLoad %float %29",
               "OpStore %param %30",
         "%31 = OpFunctionCall %float %foo_f1_ %param",
         "%32 = OpFOrdGreaterThan %bool %31 %float_2",
               "OpSelectionMerge %33 None",
               "OpBranchConditional %32 %34 %33",
         "%34 = OpLabel",
         "%35 = OpAccessChain %_ptr_Function_float %color %uint_1",
         "%36 = OpLoad %float %35",
               "OpStore %param_0 %36",
         "%37 = OpFunctionCall %float %foo_f1_ %param_0",
         "%38 = OpFOrdGreaterThan %bool %37 %float_2",
               "OpBranch %33",
         "%33 = OpLabel",
         "%39 = OpPhi %bool %32 %27 %38 %34",
               "OpSelectionMerge %40 None",
               "OpBranchConditional %39 %41 %40",
         "%41 = OpLabel",
               "OpStore %color %25",
               "OpBranch %40",
         "%40 = OpLabel",
         "%42 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %42",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %12",
         "%27 = OpLabel",
         "%62 = OpVariable %_ptr_Function_float Function",
         "%63 = OpVariable %_ptr_Function_float Function",
         "%52 = OpVariable %_ptr_Function_float Function",
         "%53 = OpVariable %_ptr_Function_float Function",
      "%color = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_float Function",
    "%param_0 = OpVariable %_ptr_Function_float Function",
         "%28 = OpLoad %v4float %BaseColor",
               "OpStore %color %28",
         "%29 = OpAccessChain %_ptr_Function_float %color %uint_0",
         "%30 = OpLoad %float %29",
               "OpStore %param %30",
         "%54 = OpLoad %float %param",
               "OpStore %52 %54",
         "%55 = OpLoad %float %52",
         "%56 = OpFOrdLessThan %bool %55 %float_0",
               "OpSelectionMerge %57 None",
               "OpBranchConditional %56 %58 %57",
         "%58 = OpLabel",
         "%59 = OpLoad %float %52",
         "%60 = OpFNegate %float %59",
               "OpStore %52 %60",
               "OpBranch %57",
         "%57 = OpLabel",
         "%61 = OpLoad %float %52",
               "OpStore %53 %61",
         "%31 = OpLoad %float %53",
         "%32 = OpFOrdGreaterThan %bool %31 %float_2",
               "OpSelectionMerge %33 None",
               "OpBranchConditional %32 %34 %33",
         "%34 = OpLabel",
         "%35 = OpAccessChain %_ptr_Function_float %color %uint_1",
         "%36 = OpLoad %float %35",
               "OpStore %param_0 %36",
         "%64 = OpLoad %float %param_0",
               "OpStore %62 %64",
         "%65 = OpLoad %float %62",
         "%66 = OpFOrdLessThan %bool %65 %float_0",
               "OpSelectionMerge %67 None",
               "OpBranchConditional %66 %68 %67",
         "%68 = OpLabel",
         "%69 = OpLoad %float %62",
         "%70 = OpFNegate %float %69",
               "OpStore %62 %70",
               "OpBranch %67",
         "%67 = OpLabel",
         "%71 = OpLoad %float %62",
               "OpStore %63 %71",
         "%37 = OpLoad %float %63",
         "%38 = OpFOrdGreaterThan %bool %37 %float_2",
               "OpBranch %33",
         "%33 = OpLabel",
         "%39 = OpPhi %bool %32 %57 %38 %67",
               "OpSelectionMerge %40 None",
               "OpBranchConditional %39 %41 %40",
         "%41 = OpLabel",
               "OpStore %color %25",
               "OpBranch %40",
         "%40 = OpLabel",
         "%42 = OpLoad %v4float %color",
               "OpStore %gl_FragColor %42",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, OpSampledImageOutOfBlock) {
  // #version 450
  //
  // uniform texture2D t2D;
  // uniform sampler samp;
  // out vec4 FragColor;
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     float r = bar.x;
  //     if (r < 0.0)
  //         r = -r;
  //     return r;
  // }
  //
  // void main()
  // {
  //     vec4 color1 = texture(sampler2D(t2D, samp), vec2(1.0));
  //     vec4 color2 = vec4(foo(BaseColor));
  //     vec4 color3 = texture(sampler2D(t2D, samp), vec2(0.5));
  //     FragColor = (color1 + color2 + color3)/3;
  // }
  //
  // Note: the before SPIR-V will need to be edited to create a use of
  // the OpSampledImage across the function call.
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 450",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %r \"r\"",
               "OpName %color1 \"color1\"",
               "OpName %t2D \"t2D\"",
               "OpName %samp \"samp\"",
               "OpName %color2 \"color2\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %color3 \"color3\"",
               "OpName %FragColor \"FragColor\"",
               "OpDecorate %t2D DescriptorSet 0",
               "OpDecorate %samp DescriptorSet 0",
       "%void = OpTypeVoid",
         "%15 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%19 = OpTypeFunction %float %_ptr_Function_v4float",
"%_ptr_Function_float = OpTypePointer Function %float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
    "%float_0 = OpConstant %float 0",
       "%bool = OpTypeBool",
         "%25 = OpTypeImage %float 2D 0 0 0 1 Unknown",
"%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25",
        "%t2D = OpVariable %_ptr_UniformConstant_25 UniformConstant",
         "%27 = OpTypeSampler",
"%_ptr_UniformConstant_27 = OpTypePointer UniformConstant %27",
       "%samp = OpVariable %_ptr_UniformConstant_27 UniformConstant",
         "%29 = OpTypeSampledImage %25",
    "%v2float = OpTypeVector %float 2",
    "%float_1 = OpConstant %float 1",
         "%32 = OpConstantComposite %v2float %float_1 %float_1",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
  "%float_0_5 = OpConstant %float 0.5",
         "%35 = OpConstantComposite %v2float %float_0_5 %float_0_5",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
  "%FragColor = OpVariable %_ptr_Output_v4float Output",
    "%float_3 = OpConstant %float 3",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %float None %19",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%56 = OpLabel",
          "%r = OpVariable %_ptr_Function_float Function",
         "%57 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%58 = OpLoad %float %57",
               "OpStore %r %58",
         "%59 = OpLoad %float %r",
         "%60 = OpFOrdLessThan %bool %59 %float_0",
               "OpSelectionMerge %61 None",
               "OpBranchConditional %60 %62 %61",
         "%62 = OpLabel",
         "%63 = OpLoad %float %r",
         "%64 = OpFNegate %float %63",
               "OpStore %r %64",
               "OpBranch %61",
         "%61 = OpLabel",
         "%65 = OpLoad %float %r",
               "OpReturnValue %65",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %15",
         "%38 = OpLabel",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%39 = OpLoad %25 %t2D",
         "%40 = OpLoad %27 %samp",
         "%41 = OpSampledImage %29 %39 %40",
         "%42 = OpImageSampleImplicitLod %v4float %41 %32",
               "OpStore %color1 %42",
         "%43 = OpLoad %v4float %BaseColor",
               "OpStore %param %43",
         "%44 = OpFunctionCall %float %foo_vf4_ %param",
         "%45 = OpCompositeConstruct %v4float %44 %44 %44 %44",
               "OpStore %color2 %45",
         "%46 = OpLoad %25 %t2D",
         "%47 = OpLoad %27 %samp",
         "%48 = OpImageSampleImplicitLod %v4float %41 %35",
               "OpStore %color3 %48",
         "%49 = OpLoad %v4float %color1",
         "%50 = OpLoad %v4float %color2",
         "%51 = OpFAdd %v4float %49 %50",
         "%52 = OpLoad %v4float %color3",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%55 = OpFDiv %v4float %53 %54",
               "OpStore %FragColor %55",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %15",
         "%38 = OpLabel",
         "%66 = OpVariable %_ptr_Function_float Function",
         "%67 = OpVariable %_ptr_Function_float Function",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%39 = OpLoad %25 %t2D",
         "%40 = OpLoad %27 %samp",
         "%41 = OpSampledImage %29 %39 %40",
         "%42 = OpImageSampleImplicitLod %v4float %41 %32",
               "OpStore %color1 %42",
         "%43 = OpLoad %v4float %BaseColor",
               "OpStore %param %43",
         "%68 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%69 = OpLoad %float %68",
               "OpStore %66 %69",
         "%70 = OpLoad %float %66",
         "%71 = OpFOrdLessThan %bool %70 %float_0",
               "OpSelectionMerge %72 None",
               "OpBranchConditional %71 %73 %72",
         "%73 = OpLabel",
         "%74 = OpLoad %float %66",
         "%75 = OpFNegate %float %74",
               "OpStore %66 %75",
               "OpBranch %72",
         "%72 = OpLabel",
         "%76 = OpLoad %float %66",
               "OpStore %67 %76",
         "%44 = OpLoad %float %67",
         "%45 = OpCompositeConstruct %v4float %44 %44 %44 %44",
               "OpStore %color2 %45",
         "%46 = OpLoad %25 %t2D",
         "%47 = OpLoad %27 %samp",
         "%77 = OpSampledImage %29 %39 %40",
         "%48 = OpImageSampleImplicitLod %v4float %77 %35",
               "OpStore %color3 %48",
         "%49 = OpLoad %v4float %color1",
         "%50 = OpLoad %v4float %color2",
         "%51 = OpFAdd %v4float %49 %50",
         "%52 = OpLoad %v4float %color3",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%55 = OpFDiv %v4float %53 %54",
               "OpStore %FragColor %55",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, OpImageOutOfBlock) {
  // #version 450
  //
  // uniform texture2D t2D;
  // uniform sampler samp;
  // uniform sampler samp2;
  //
  // out vec4 FragColor;
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     float r = bar.x;
  //     if (r < 0.0)
  //         r = -r;
  //     return r;
  // }
  //
  // void main()
  // {
  //     vec4 color1 = texture(sampler2D(t2D, samp), vec2(1.0));
  //     vec4 color2 = vec4(foo(BaseColor));
  //     vec4 color3 = texture(sampler2D(t2D, samp2), vec2(0.5));
  //     FragColor = (color1 + color2 + color3)/3;
  // }
  // Note: the before SPIR-V will need to be edited to create an OpImage
  // from the first OpSampledImage, place it before the call and use it
  // in the second OpSampledImage following the call.
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 450",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %r \"r\"",
               "OpName %color1 \"color1\"",
               "OpName %t2D \"t2D\"",
               "OpName %samp \"samp\"",
               "OpName %color2 \"color2\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %color3 \"color3\"",
               "OpName %samp2 \"samp2\"",
               "OpName %FragColor \"FragColor\"",
               "OpDecorate %t2D DescriptorSet 0",
               "OpDecorate %samp DescriptorSet 0",
               "OpDecorate %samp2 DescriptorSet 0",
       "%void = OpTypeVoid",
         "%16 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%20 = OpTypeFunction %float %_ptr_Function_v4float",
"%_ptr_Function_float = OpTypePointer Function %float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
    "%float_0 = OpConstant %float 0",
       "%bool = OpTypeBool",
         "%26 = OpTypeImage %float 2D 0 0 0 1 Unknown",
"%_ptr_UniformConstant_26 = OpTypePointer UniformConstant %26",
        "%t2D = OpVariable %_ptr_UniformConstant_26 UniformConstant",
         "%28 = OpTypeSampler",
"%_ptr_UniformConstant_28 = OpTypePointer UniformConstant %28",
       "%samp = OpVariable %_ptr_UniformConstant_28 UniformConstant",
         "%30 = OpTypeSampledImage %26",
    "%v2float = OpTypeVector %float 2",
    "%float_1 = OpConstant %float 1",
         "%33 = OpConstantComposite %v2float %float_1 %float_1",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
      "%samp2 = OpVariable %_ptr_UniformConstant_28 UniformConstant",
  "%float_0_5 = OpConstant %float 0.5",
         "%36 = OpConstantComposite %v2float %float_0_5 %float_0_5",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
  "%FragColor = OpVariable %_ptr_Output_v4float Output",
    "%float_3 = OpConstant %float 3",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %float None %20",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%58 = OpLabel",
          "%r = OpVariable %_ptr_Function_float Function",
         "%59 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%60 = OpLoad %float %59",
               "OpStore %r %60",
         "%61 = OpLoad %float %r",
         "%62 = OpFOrdLessThan %bool %61 %float_0",
               "OpSelectionMerge %63 None",
               "OpBranchConditional %62 %64 %63",
         "%64 = OpLabel",
         "%65 = OpLoad %float %r",
         "%66 = OpFNegate %float %65",
               "OpStore %r %66",
               "OpBranch %63",
         "%63 = OpLabel",
         "%67 = OpLoad %float %r",
               "OpReturnValue %67",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %16",
         "%39 = OpLabel",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%40 = OpLoad %26 %t2D",
         "%41 = OpLoad %28 %samp",
         "%42 = OpSampledImage %30 %40 %41",
         "%43 = OpImageSampleImplicitLod %v4float %42 %33",
         "%44 = OpImage %26 %42",
         "%45 = OpLoad %28 %samp2",
               "OpStore %color1 %43",
         "%46 = OpLoad %v4float %BaseColor",
               "OpStore %param %46",
         "%47 = OpFunctionCall %float %foo_vf4_ %param",
         "%48 = OpCompositeConstruct %v4float %47 %47 %47 %47",
               "OpStore %color2 %48",
         "%49 = OpSampledImage %30 %44 %45",
         "%50 = OpImageSampleImplicitLod %v4float %49 %36",
               "OpStore %color3 %50",
         "%51 = OpLoad %v4float %color1",
         "%52 = OpLoad %v4float %color2",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpLoad %v4float %color3",
         "%55 = OpFAdd %v4float %53 %54",
         "%56 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%57 = OpFDiv %v4float %55 %56",
               "OpStore %FragColor %57",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %16",
         "%39 = OpLabel",
         "%68 = OpVariable %_ptr_Function_float Function",
         "%69 = OpVariable %_ptr_Function_float Function",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%40 = OpLoad %26 %t2D",
         "%41 = OpLoad %28 %samp",
         "%42 = OpSampledImage %30 %40 %41",
         "%43 = OpImageSampleImplicitLod %v4float %42 %33",
         "%44 = OpImage %26 %42",
         "%45 = OpLoad %28 %samp2",
               "OpStore %color1 %43",
         "%46 = OpLoad %v4float %BaseColor",
               "OpStore %param %46",
         "%70 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%71 = OpLoad %float %70",
               "OpStore %68 %71",
         "%72 = OpLoad %float %68",
         "%73 = OpFOrdLessThan %bool %72 %float_0",
               "OpSelectionMerge %74 None",
               "OpBranchConditional %73 %75 %74",
         "%75 = OpLabel",
         "%76 = OpLoad %float %68",
         "%77 = OpFNegate %float %76",
               "OpStore %68 %77",
               "OpBranch %74",
         "%74 = OpLabel",
         "%78 = OpLoad %float %68",
               "OpStore %69 %78",
         "%47 = OpLoad %float %69",
         "%48 = OpCompositeConstruct %v4float %47 %47 %47 %47",
               "OpStore %color2 %48",
         "%79 = OpSampledImage %30 %40 %41",
         "%80 = OpImage %26 %79",
         "%49 = OpSampledImage %30 %80 %45",
         "%50 = OpImageSampleImplicitLod %v4float %49 %36",
               "OpStore %color3 %50",
         "%51 = OpLoad %v4float %color1",
         "%52 = OpLoad %v4float %color2",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpLoad %v4float %color3",
         "%55 = OpFAdd %v4float %53 %54",
         "%56 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%57 = OpFDiv %v4float %55 %56",
               "OpStore %FragColor %57",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, OpImageAndOpSampledImageOutOfBlock) {
  // #version 450
  //
  // uniform texture2D t2D;
  // uniform sampler samp;
  // uniform sampler samp2;
  //
  // out vec4 FragColor;
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     float r = bar.x;
  //     if (r < 0.0)
  //         r = -r;
  //     return r;
  // }
  //
  // void main()
  // {
  //     vec4 color1 = texture(sampler2D(t2D, samp), vec2(1.0));
  //     vec4 color2 = vec4(foo(BaseColor));
  //     vec4 color3 = texture(sampler2D(t2D, samp2), vec2(0.5));
  //     FragColor = (color1 + color2 + color3)/3;
  // }
  // Note: the before SPIR-V will need to be edited to create an OpImage
  // and subsequent OpSampledImage that is used across the function call.
  const std::vector<const char*> predefs = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\" %BaseColor %FragColor",
               "OpExecutionMode %main OriginUpperLeft",
               "OpSource GLSL 450",
               "OpName %main \"main\"",
               "OpName %foo_vf4_ \"foo(vf4;\"",
               "OpName %bar \"bar\"",
               "OpName %r \"r\"",
               "OpName %color1 \"color1\"",
               "OpName %t2D \"t2D\"",
               "OpName %samp \"samp\"",
               "OpName %color2 \"color2\"",
               "OpName %BaseColor \"BaseColor\"",
               "OpName %param \"param\"",
               "OpName %color3 \"color3\"",
               "OpName %samp2 \"samp2\"",
               "OpName %FragColor \"FragColor\"",
               "OpDecorate %t2D DescriptorSet 0",
               "OpDecorate %samp DescriptorSet 0",
               "OpDecorate %samp2 DescriptorSet 0",
       "%void = OpTypeVoid",
         "%16 = OpTypeFunction %void",
      "%float = OpTypeFloat 32",
    "%v4float = OpTypeVector %float 4",
"%_ptr_Function_v4float = OpTypePointer Function %v4float",
         "%20 = OpTypeFunction %float %_ptr_Function_v4float",
"%_ptr_Function_float = OpTypePointer Function %float",
       "%uint = OpTypeInt 32 0",
     "%uint_0 = OpConstant %uint 0",
    "%float_0 = OpConstant %float 0",
       "%bool = OpTypeBool",
         "%26 = OpTypeImage %float 2D 0 0 0 1 Unknown",
"%_ptr_UniformConstant_26 = OpTypePointer UniformConstant %26",
        "%t2D = OpVariable %_ptr_UniformConstant_26 UniformConstant",
         "%28 = OpTypeSampler",
"%_ptr_UniformConstant_28 = OpTypePointer UniformConstant %28",
       "%samp = OpVariable %_ptr_UniformConstant_28 UniformConstant",
         "%30 = OpTypeSampledImage %26",
    "%v2float = OpTypeVector %float 2",
    "%float_1 = OpConstant %float 1",
         "%33 = OpConstantComposite %v2float %float_1 %float_1",
"%_ptr_Input_v4float = OpTypePointer Input %v4float",
  "%BaseColor = OpVariable %_ptr_Input_v4float Input",
      "%samp2 = OpVariable %_ptr_UniformConstant_28 UniformConstant",
  "%float_0_5 = OpConstant %float 0.5",
         "%36 = OpConstantComposite %v2float %float_0_5 %float_0_5",
"%_ptr_Output_v4float = OpTypePointer Output %v4float",
  "%FragColor = OpVariable %_ptr_Output_v4float Output",
    "%float_3 = OpConstant %float 3",
      // clang-format on
  };

  const std::vector<const char*> nonEntryFuncs = {
      // clang-format off
   "%foo_vf4_ = OpFunction %float None %20",
        "%bar = OpFunctionParameter %_ptr_Function_v4float",
         "%58 = OpLabel",
          "%r = OpVariable %_ptr_Function_float Function",
         "%59 = OpAccessChain %_ptr_Function_float %bar %uint_0",
         "%60 = OpLoad %float %59",
               "OpStore %r %60",
         "%61 = OpLoad %float %r",
         "%62 = OpFOrdLessThan %bool %61 %float_0",
               "OpSelectionMerge %63 None",
               "OpBranchConditional %62 %64 %63",
         "%64 = OpLabel",
         "%65 = OpLoad %float %r",
         "%66 = OpFNegate %float %65",
               "OpStore %r %66",
               "OpBranch %63",
         "%63 = OpLabel",
         "%67 = OpLoad %float %r",
               "OpReturnValue %67",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> before = {
      // clang-format off
       "%main = OpFunction %void None %16",
         "%39 = OpLabel",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%40 = OpLoad %26 %t2D",
         "%41 = OpLoad %28 %samp",
         "%42 = OpSampledImage %30 %40 %41",
         "%43 = OpImageSampleImplicitLod %v4float %42 %33",
         "%44 = OpImage %26 %42",
         "%45 = OpLoad %28 %samp2",
         "%46 = OpSampledImage %30 %44 %45",
               "OpStore %color1 %43",
         "%47 = OpLoad %v4float %BaseColor",
               "OpStore %param %47",
         "%48 = OpFunctionCall %float %foo_vf4_ %param",
         "%49 = OpCompositeConstruct %v4float %48 %48 %48 %48",
               "OpStore %color2 %49",
         "%50 = OpImageSampleImplicitLod %v4float %46 %36",
               "OpStore %color3 %50",
         "%51 = OpLoad %v4float %color1",
         "%52 = OpLoad %v4float %color2",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpLoad %v4float %color3",
         "%55 = OpFAdd %v4float %53 %54",
         "%56 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%57 = OpFDiv %v4float %55 %56",
               "OpStore %FragColor %57",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  const std::vector<const char*> after = {
      // clang-format off
       "%main = OpFunction %void None %16",
         "%39 = OpLabel",
         "%68 = OpVariable %_ptr_Function_float Function",
         "%69 = OpVariable %_ptr_Function_float Function",
     "%color1 = OpVariable %_ptr_Function_v4float Function",
     "%color2 = OpVariable %_ptr_Function_v4float Function",
      "%param = OpVariable %_ptr_Function_v4float Function",
     "%color3 = OpVariable %_ptr_Function_v4float Function",
         "%40 = OpLoad %26 %t2D",
         "%41 = OpLoad %28 %samp",
         "%42 = OpSampledImage %30 %40 %41",
         "%43 = OpImageSampleImplicitLod %v4float %42 %33",
         "%44 = OpImage %26 %42",
         "%45 = OpLoad %28 %samp2",
         "%46 = OpSampledImage %30 %44 %45",
               "OpStore %color1 %43",
         "%47 = OpLoad %v4float %BaseColor",
               "OpStore %param %47",
         "%70 = OpAccessChain %_ptr_Function_float %param %uint_0",
         "%71 = OpLoad %float %70",
               "OpStore %68 %71",
         "%72 = OpLoad %float %68",
         "%73 = OpFOrdLessThan %bool %72 %float_0",
               "OpSelectionMerge %74 None",
               "OpBranchConditional %73 %75 %74",
         "%75 = OpLabel",
         "%76 = OpLoad %float %68",
         "%77 = OpFNegate %float %76",
               "OpStore %68 %77",
               "OpBranch %74",
         "%74 = OpLabel",
         "%78 = OpLoad %float %68",
               "OpStore %69 %78",
         "%48 = OpLoad %float %69",
         "%49 = OpCompositeConstruct %v4float %48 %48 %48 %48",
               "OpStore %color2 %49",
         "%79 = OpSampledImage %30 %40 %41",
         "%80 = OpImage %26 %79",
         "%81 = OpSampledImage %30 %80 %45",
         "%50 = OpImageSampleImplicitLod %v4float %81 %36",
               "OpStore %color3 %50",
         "%51 = OpLoad %v4float %color1",
         "%52 = OpLoad %v4float %color2",
         "%53 = OpFAdd %v4float %51 %52",
         "%54 = OpLoad %v4float %color3",
         "%55 = OpFAdd %v4float %53 %54",
         "%56 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3",
         "%57 = OpFDiv %v4float %55 %56",
               "OpStore %FragColor %57",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };
  SinglePassRunAndCheck<InlineExhaustivePass>(
      JoinAllInsts(Concat(Concat(predefs, before), nonEntryFuncs)),
      JoinAllInsts(Concat(Concat(predefs, after), nonEntryFuncs)),
      /* skip_nop = */ false, /* do_validate = */ true);
}

TEST_F(InlineTest, EarlyReturnFunctionInlined) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     if (bar.x < 0.0)
  //         return 0.0;
  //     return bar.x;
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_ "foo(vf4;"
OpName %bar "bar"
OpName %color "color"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %float %_ptr_Function_v4float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string nonEntryFuncs =
      R"(%foo_vf4_ = OpFunction %float None %14
%bar = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%28 = OpAccessChain %_ptr_Function_float %bar %uint_0
%29 = OpLoad %float %28
%30 = OpFOrdLessThan %bool %29 %float_0
OpSelectionMerge %31 None
OpBranchConditional %30 %32 %31
%32 = OpLabel
OpReturnValue %float_0
%31 = OpLabel
%33 = OpAccessChain %_ptr_Function_float %bar %uint_0
%34 = OpLoad %float %33
OpReturnValue %34
OpFunctionEnd
)";

  const std::string before =
      R"(%main = OpFunction %void None %10
%22 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %param %23
%24 = OpFunctionCall %float %foo_vf4_ %param
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %color %25
%26 = OpLoad %v4float %color
OpStore %gl_FragColor %26
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%false = OpConstantFalse %bool
%main = OpFunction %void None %10
%22 = OpLabel
%35 = OpVariable %_ptr_Function_float Function
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %param %23
OpBranch %36
%36 = OpLabel
OpLoopMerge %37 %38 None
OpBranch %39
%39 = OpLabel
%40 = OpAccessChain %_ptr_Function_float %param %uint_0
%41 = OpLoad %float %40
%42 = OpFOrdLessThan %bool %41 %float_0
OpSelectionMerge %43 None
OpBranchConditional %42 %44 %43
%44 = OpLabel
OpStore %35 %float_0
OpBranch %37
%43 = OpLabel
%45 = OpAccessChain %_ptr_Function_float %param %uint_0
%46 = OpLoad %float %45
OpStore %35 %46
OpBranch %37
%38 = OpLabel
OpBranchConditional %false %36 %37
%37 = OpLabel
%24 = OpLoad %float %35
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %color %25
%26 = OpLoad %v4float %color
OpStore %gl_FragColor %26
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + before + nonEntryFuncs,
                                              predefs + after + nonEntryFuncs,
                                              false, true);
}

TEST_F(InlineTest, EarlyReturnNotAppearingLastInFunctionInlined) {
  // Example from https://github.com/KhronosGroup/SPIRV-Tools/issues/755
  //
  // Original example is derived from:
  //
  // #version 450
  //
  // float foo() {
  //     if (true) {
  //     }
  // }
  //
  // void main() { foo(); }
  //
  // But the order of basic blocks in foo is changed so that the return
  // block is listed second-last.  There is only one return in the callee
  // but it does not appear last.

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpSource GLSL 450
OpName %main "main"
OpName %foo_ "foo("
%void = OpTypeVoid
%4 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
)";

  const std::string nonEntryFuncs =
      R"(%foo_ = OpFunction %void None %4
%7 = OpLabel
OpSelectionMerge %8 None
OpBranchConditional %true %9 %8
%8 = OpLabel
OpReturn
%9 = OpLabel
OpBranch %8
OpFunctionEnd
)";

  const std::string before =
      R"(%main = OpFunction %void None %4
%10 = OpLabel
%11 = OpFunctionCall %void %foo_
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %4
%10 = OpLabel
OpSelectionMerge %12 None
OpBranchConditional %true %13 %12
%12 = OpLabel
OpBranch %14
%13 = OpLabel
OpBranch %12
%14 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, ForwardReferencesInPhiInlined) {
  // The basic structure of the test case is like this:
  //
  // int foo() {
  //   int result = 1;
  //   if (true) {
  //      result = 1;
  //   }
  //   return result;
  // }
  //
  // void main() {
  //  int x = foo();
  // }
  //
  // but with modifications: Using Phi instead of load/store, and the
  // return block in foo appears before the "then" block.

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpSource GLSL 450
OpName %main "main"
OpName %foo_ "foo("
OpName %x "x"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%int = OpTypeInt 32 1
%8 = OpTypeFunction %int
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
)";

  const std::string nonEntryFuncs =
      R"(%foo_ = OpFunction %int None %8
%13 = OpLabel
%14 = OpCopyObject %int %int_0
OpSelectionMerge %15 None
OpBranchConditional %true %16 %15
%15 = OpLabel
%17 = OpPhi %int %14 %13 %18 %16
OpReturnValue %17
%16 = OpLabel
%18 = OpCopyObject %int %int_0
OpBranch %15
OpFunctionEnd
)";

  const std::string before =
      R"(%main = OpFunction %void None %6
%19 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%20 = OpFunctionCall %int %foo_
OpStore %x %20
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %6
%19 = OpLabel
%21 = OpVariable %_ptr_Function_int Function
%x = OpVariable %_ptr_Function_int Function
%22 = OpCopyObject %int %int_0
OpSelectionMerge %23 None
OpBranchConditional %true %24 %23
%23 = OpLabel
%26 = OpPhi %int %22 %19 %25 %24
OpStore %21 %26
OpBranch %27
%24 = OpLabel
%25 = OpCopyObject %int %int_0
OpBranch %23
%27 = OpLabel
%20 = OpLoad %int %21
OpStore %x %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, EarlyReturnInLoopIsNotInlined) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     while (true) {
  //         if (bar.x < 0.0)
  //             return 0.0;
  //         return bar.x;
  //     }
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_ "foo(vf4;"
OpName %bar "bar"
OpName %color "color"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %float %_ptr_Function_v4float
%bool = OpTypeBool
%true = OpConstantTrue %bool
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %10
%23 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %v4float %BaseColor
OpStore %param %24
%25 = OpFunctionCall %float %foo_vf4_ %param
%26 = OpCompositeConstruct %v4float %25 %25 %25 %25
OpStore %color %26
%27 = OpLoad %v4float %color
OpStore %gl_FragColor %27
OpReturn
OpFunctionEnd
%foo_vf4_ = OpFunction %float None %14
%bar = OpFunctionParameter %_ptr_Function_v4float
%28 = OpLabel
OpBranch %29
%29 = OpLabel
OpLoopMerge %30 %31 None
OpBranch %32
%32 = OpLabel
OpBranchConditional %true %33 %30
%33 = OpLabel
%34 = OpAccessChain %_ptr_Function_float %bar %uint_0
%35 = OpLoad %float %34
%36 = OpFOrdLessThan %bool %35 %float_0
OpSelectionMerge %37 None
OpBranchConditional %36 %38 %37
%38 = OpLabel
OpReturnValue %float_0
%37 = OpLabel
%39 = OpAccessChain %_ptr_Function_float %bar %uint_0
%40 = OpLoad %float %39
OpReturnValue %40
%31 = OpLabel
OpBranch %29
%30 = OpLabel
%41 = OpUndef %float
OpReturnValue %41
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(assembly, assembly, false, true);
}

TEST_F(InlineTest, ExternalFunctionIsNotInlined) {
  // In particular, don't crash.
  // See report https://github.com/KhronosGroup/SPIRV-Tools/issues/605
  const std::string assembly =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %1 "entry_pt"
OpDecorate %2 LinkageAttributes "external" Import
%void = OpTypeVoid
%4 = OpTypeFunction %void
%2 = OpFunction %void None %4
OpFunctionEnd
%1 = OpFunction %void None %4
%5 = OpLabel
%6 = OpFunctionCall %void %2
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(assembly, assembly, false, true);
}

TEST_F(InlineTest, SingleBlockLoopCallsMultiBlockCallee) {
  // Example from https://github.com/KhronosGroup/SPIRV-Tools/issues/787
  //
  // CFG structure is:
  //    foo:
  //       fooentry -> fooexit
  //
  //    main:
  //       entry -> loop
  //       loop -> loop, merge
  //         loop calls foo()
  //       merge
  //
  // Since the callee has multiple blocks, it will split the calling block
  // into at least two, resulting in a new "back-half" block that contains
  // the instructions after the inlined function call.  If the calling block
  // has an OpLoopMerge that points back to the calling block itself, then
  // the OpLoopMerge can't remain in the back-half block, but must be
  // moved to the end of the original calling block, and it continue target
  // operand updated to point to the back-half block.

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
)";

  const std::string nonEntryFuncs =
      R"(%5 = OpTypeFunction %void
%6 = OpFunction %void None %5
%7 = OpLabel
OpBranch %8
%8 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %5
%9 = OpLabel
OpBranch %10
%10 = OpLabel
%11 = OpFunctionCall %void %6
OpLoopMerge %12 %10 None
OpBranchConditional %true %10 %12
%12 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%1 = OpFunction %void None %5
%9 = OpLabel
OpBranch %10
%10 = OpLabel
OpLoopMerge %12 %10 None
OpBranch %13
%13 = OpLabel
OpBranchConditional %true %10 %12
%12 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, MultiBlockLoopHeaderCallsMultiBlockCallee) {
  // Like SingleBlockLoopCallsMultiBlockCallee but the loop has several
  // blocks, but the function call still occurs in the loop header.
  // Example from https://github.com/KhronosGroup/SPIRV-Tools/issues/800

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%void = OpTypeVoid
%11 = OpTypeFunction %void
)";

  const std::string nonEntryFuncs =
      R"(%12 = OpFunction %void None %11
%13 = OpLabel
%14 = OpCopyObject %int %int_1
OpBranch %15
%15 = OpLabel
%16 = OpCopyObject %int %int_2
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %11
%17 = OpLabel
OpBranch %18
%18 = OpLabel
%19 = OpCopyObject %int %int_3
%20 = OpFunctionCall %void %12
%21 = OpCopyObject %int %int_4
OpLoopMerge %22 %23 None
OpBranchConditional %true %23 %22
%23 = OpLabel
%24 = OpCopyObject %int %int_5
OpBranchConditional %true %18 %22
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%1 = OpFunction %void None %11
%17 = OpLabel
OpBranch %18
%18 = OpLabel
%19 = OpCopyObject %int %int_3
%25 = OpCopyObject %int %int_1
OpLoopMerge %22 %23 None
OpBranch %26
%26 = OpLabel
%27 = OpCopyObject %int %int_2
%21 = OpCopyObject %int %int_4
OpBranchConditional %true %23 %22
%23 = OpLabel
%24 = OpCopyObject %int %int_5
OpBranchConditional %true %18 %22
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, SingleBlockLoopCallsMultiBlockCalleeHavingSelectionMerge) {
  // This is similar to SingleBlockLoopCallsMultiBlockCallee except
  // that calleee block also has a merge instruction in its first block.
  // That merge instruction must be an OpSelectionMerge (because the entry
  // block of a function can't be the header of a loop since the entry
  // block can't be the target of a branch).
  //
  // In this case the OpLoopMerge can't be placed in the same block as
  // the OpSelectionMerge, so inlining must create a new block to contain
  // the callee contents.
  //
  // Additionally, we have two dummy OpCopyObject instructions to prove that
  // the OpLoopMerge is moved to the right location.
  //
  // Also ensure that OpPhis within the cloned callee code are valid.
  // We need to test that the predecessor blocks are remapped correctly so that
  // dominance rules are satisfied

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%void = OpTypeVoid
%6 = OpTypeFunction %void
)";

  // This callee has multiple blocks, and an OpPhi in the last block
  // that references a value from the first block.  This tests that
  // cloned block IDs are remapped appropriately.  The OpPhi dominance
  // requires that the remapped %9 must be in a block that dominates
  // the remapped %8.
  const std::string nonEntryFuncs =
      R"(%7 = OpFunction %void None %6
%8 = OpLabel
%9 = OpCopyObject %bool %true
OpSelectionMerge %10 None
OpBranchConditional %true %10 %10
%10 = OpLabel
%11 = OpPhi %bool %9 %8
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %6
%12 = OpLabel
OpBranch %13
%13 = OpLabel
%14 = OpCopyObject %bool %false
%15 = OpFunctionCall %void %7
OpLoopMerge %16 %13 None
OpBranchConditional %true %13 %16
%16 = OpLabel
OpReturn
OpFunctionEnd
)";

  // Note the remapped Phi uses %17 as the parent instead
  // of %13, demonstrating that the parent block has been remapped
  // correctly.
  const std::string after =
      R"(%1 = OpFunction %void None %6
%12 = OpLabel
OpBranch %13
%13 = OpLabel
%14 = OpCopyObject %bool %false
OpLoopMerge %16 %13 None
OpBranch %17
%17 = OpLabel
%18 = OpCopyObject %bool %true
OpSelectionMerge %19 None
OpBranchConditional %true %19 %19
%19 = OpLabel
%20 = OpPhi %bool %18 %17
OpBranchConditional %true %13 %16
%16 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest,
       MultiBlockLoopHeaderCallsFromToMultiBlockCalleeHavingSelectionMerge) {
  // This is similar to SingleBlockLoopCallsMultiBlockCalleeHavingSelectionMerge
  // but the call is in the header block of a multi block loop.

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%void = OpTypeVoid
%11 = OpTypeFunction %void
)";

  const std::string nonEntryFuncs =
      R"(%12 = OpFunction %void None %11
%13 = OpLabel
%14 = OpCopyObject %int %int_1
OpSelectionMerge %15 None
OpBranchConditional %true %15 %15
%15 = OpLabel
%16 = OpCopyObject %int %int_2
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %11
%17 = OpLabel
OpBranch %18
%18 = OpLabel
%19 = OpCopyObject %int %int_3
%20 = OpFunctionCall %void %12
%21 = OpCopyObject %int %int_4
OpLoopMerge %22 %23 None
OpBranchConditional %true %23 %22
%23 = OpLabel
%24 = OpCopyObject %int %int_5
OpBranchConditional %true %18 %22
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%1 = OpFunction %void None %11
%17 = OpLabel
OpBranch %18
%18 = OpLabel
%19 = OpCopyObject %int %int_3
OpLoopMerge %22 %23 None
OpBranch %25
%25 = OpLabel
%26 = OpCopyObject %int %int_1
OpSelectionMerge %27 None
OpBranchConditional %true %27 %27
%27 = OpLabel
%28 = OpCopyObject %int %int_2
%21 = OpCopyObject %int %int_4
OpBranchConditional %true %23 %22
%23 = OpLabel
%24 = OpCopyObject %int %int_5
OpBranchConditional %true %18 %22
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(
    InlineTest,
    SingleBlockLoopCallsMultiBlockCalleeHavingSelectionMergeAndMultiReturns) {
  // This is similar to SingleBlockLoopCallsMultiBlockCalleeHavingSelectionMerge
  // except that in addition to starting with a selection header, the
  // callee also has multi returns.
  //
  // So now we have to accommodate:
  // - The caller's OpLoopMerge (which must move to the first block)
  // - The single-trip loop to wrap the multi returns, and
  // - The callee's selection merge in its first block.
  // Each of these must go into their own blocks.

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%bool = OpTypeBool
%int = OpTypeInt 32 1
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%void = OpTypeVoid
%12 = OpTypeFunction %void
)";

  const std::string nonEntryFuncs =
      R"(%13 = OpFunction %void None %12
%14 = OpLabel
%15 = OpCopyObject %int %int_0
OpReturn
%16 = OpLabel
%17 = OpCopyObject %int %int_1
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %12
%18 = OpLabel
OpBranch %19
%19 = OpLabel
%20 = OpCopyObject %int %int_2
%21 = OpFunctionCall %void %13
%22 = OpCopyObject %int %int_3
OpLoopMerge %23 %19 None
OpBranchConditional %true %19 %23
%23 = OpLabel
%24 = OpCopyObject %int %int_4
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%1 = OpFunction %void None %12
%18 = OpLabel
OpBranch %19
%19 = OpLabel
%20 = OpCopyObject %int %int_2
%25 = OpCopyObject %int %int_0
OpLoopMerge %23 %19 None
OpBranch %26
%27 = OpLabel
%28 = OpCopyObject %int %int_1
OpBranch %26
%26 = OpLabel
%22 = OpCopyObject %int %int_3
OpBranchConditional %true %19 %23
%23 = OpLabel
%24 = OpCopyObject %int %int_4
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, CalleeWithMultiReturnAndPhiRequiresEntryBlockRemapping) {
  // The case from https://github.com/KhronosGroup/SPIRV-Tools/issues/790
  //
  // The callee has multiple returns, and so must be wrapped with a single-trip
  // loop.  That code must remap the callee entry block ID to the introduced
  // loop body's ID.  Otherwise you can get a dominance error in a cloned OpPhi.

  const std::string predefs =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpSource OpenCL_C 120
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%void = OpTypeVoid
%9 = OpTypeFunction %void
%bool = OpTypeBool
%false = OpConstantFalse %bool
)";

  // This callee has multiple returns, and a Phi in the second block referencing
  // a value generated in the entry block.
  const std::string nonEntryFuncs =
      R"(%12 = OpFunction %void None %9
%13 = OpLabel
%14 = OpCopyObject %int %int_0
OpBranch %15
%15 = OpLabel
%16 = OpPhi %int %14 %13
%17 = OpCopyObject %int %int_1
OpReturn
%18 = OpLabel
%19 = OpCopyObject %int %int_2
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%1 = OpFunction %void None %9
%20 = OpLabel
%21 = OpCopyObject %int %int_3
%22 = OpFunctionCall %void %12
%23 = OpCopyObject %int %int_4
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%1 = OpFunction %void None %9
%20 = OpLabel
%21 = OpCopyObject %int %int_3
%24 = OpCopyObject %int %int_0
OpBranch %25
%25 = OpLabel
%26 = OpPhi %int %24 %20
%27 = OpCopyObject %int %int_1
OpBranch %28
%29 = OpLabel
%30 = OpCopyObject %int %int_2
OpBranch %28
%28 = OpLabel
%23 = OpCopyObject %int %int_4
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + nonEntryFuncs + before,
                                              predefs + nonEntryFuncs + after,
                                              false, true);
}

TEST_F(InlineTest, NonInlinableCalleeWithSingleReturn) {
  // The case from https://github.com/KhronosGroup/SPIRV-Tools/issues/2018
  //
  // The callee has a single return, but cannot be inlined because the
  // return is inside a loop.

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_GLF_color
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
OpName %f_ "f("
OpName %i "i"
OpName %_GLF_color "_GLF_color"
OpDecorate %_GLF_color Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%9 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%bool = OpTypeBool
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_GLF_color = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%21 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
)";

  const std::string caller =
      R"(%main = OpFunction %void None %7
%22 = OpLabel
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %23
%23 = OpLabel
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_1
OpBranchConditional %28 %29 %24
%29 = OpLabel
OpStore %_GLF_color %20
%30 = OpFunctionCall %float %f_
OpBranch %25
%25 = OpLabel
%31 = OpLoad %int %i
%32 = OpIAdd %int %31 %int_1
OpStore %i %32
OpBranch %23
%24 = OpLabel
OpStore %_GLF_color %21
OpReturn
OpFunctionEnd
)";

  const std::string callee =
      R"(%f_ = OpFunction %float None %9
%33 = OpLabel
OpBranch %34
%34 = OpLabel
OpLoopMerge %35 %36 None
OpBranch %37
%37 = OpLabel
OpReturnValue %float_1
%36 = OpLabel
OpBranch %34
%35 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(
      predefs + caller + callee, predefs + caller + callee, false, true);
}

TEST_F(InlineTest, CalleeWithSingleReturnNeedsSingleTripLoopWrapper) {
  // The case from https://github.com/KhronosGroup/SPIRV-Tools/issues/2018
  //
  // The callee has a single return, but needs single-trip loop wrapper
  // to be inlined because the return is in a selection structure.

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_GLF_color
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
OpName %f_ "f("
OpName %i "i"
OpName %_GLF_color "_GLF_color"
OpDecorate %_GLF_color Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%9 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%bool = OpTypeBool
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_GLF_color = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%22 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
)";

  const std::string new_predefs =
      R"(%_ptr_Function_float = OpTypePointer Function %float
)";

  const std::string main_before =
      R"(%main = OpFunction %void None %7
%23 = OpLabel
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%28 = OpLoad %int %i
%29 = OpSLessThan %bool %28 %int_1
OpBranchConditional %29 %30 %25
%30 = OpLabel
OpStore %_GLF_color %21
%31 = OpFunctionCall %float %f_
OpBranch %26
%26 = OpLabel
%32 = OpLoad %int %i
%33 = OpIAdd %int %32 %int_1
OpStore %i %33
OpBranch %24
%25 = OpLabel
OpStore %_GLF_color %22
OpReturn
OpFunctionEnd
)";

  const std::string main_after =
      R"(%main = OpFunction %void None %7
%23 = OpLabel
%38 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%28 = OpLoad %int %i
%29 = OpSLessThan %bool %28 %int_1
OpBranchConditional %29 %30 %25
%30 = OpLabel
OpStore %_GLF_color %21
OpBranch %39
%39 = OpLabel
OpLoopMerge %40 %41 None
OpBranch %42
%42 = OpLabel
OpSelectionMerge %43 None
OpBranchConditional %true %44 %43
%44 = OpLabel
OpStore %38 %float_1
OpBranch %40
%43 = OpLabel
OpStore %38 %float_1
OpBranch %40
%41 = OpLabel
OpBranchConditional %false %39 %40
%40 = OpLabel
%31 = OpLoad %float %38
OpBranch %26
%26 = OpLabel
%32 = OpLoad %int %i
%33 = OpIAdd %int %32 %int_1
OpStore %i %33
OpBranch %24
%25 = OpLabel
OpStore %_GLF_color %22
OpReturn
OpFunctionEnd
)";

  const std::string callee =
      R"(%f_ = OpFunction %float None %9
%34 = OpLabel
OpSelectionMerge %35 None
OpBranchConditional %true %36 %35
%36 = OpLabel
OpReturnValue %float_1
%35 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(
      predefs + main_before + callee,
      predefs + new_predefs + main_after + callee, false, true);
}

TEST_F(InlineTest, Decorated1) {
  // Same test as Simple with the difference
  // that OpFAdd in the outlined function is
  // decorated with RelaxedPrecision
  // Expected result is an equal decoration
  // of the corresponding inlined instruction
  //
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     return bar.x + bar.y;
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_ "foo(vf4;"
OpName %bar "bar"
OpName %color "color"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %9 RelaxedPrecision
)";

  const std::string before =
      R"(%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %float %_ptr_Function_v4float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %11
%22 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %param %23
%24 = OpFunctionCall %float %foo_vf4_ %param
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %color %25
%26 = OpLoad %v4float %color
OpStore %gl_FragColor %26
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpDecorate %37 RelaxedPrecision
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %float %_ptr_Function_v4float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %11
%22 = OpLabel
%32 = OpVariable %_ptr_Function_float Function
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %param %23
%33 = OpAccessChain %_ptr_Function_float %param %uint_0
%34 = OpLoad %float %33
%35 = OpAccessChain %_ptr_Function_float %param %uint_1
%36 = OpLoad %float %35
%37 = OpFAdd %float %34 %36
OpStore %32 %37
%24 = OpLoad %float %32
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %color %25
%26 = OpLoad %v4float %color
OpStore %gl_FragColor %26
OpReturn
OpFunctionEnd
)";

  const std::string nonEntryFuncs =
      R"(%foo_vf4_ = OpFunction %float None %15
%bar = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%28 = OpAccessChain %_ptr_Function_float %bar %uint_0
%29 = OpLoad %float %28
%30 = OpAccessChain %_ptr_Function_float %bar %uint_1
%31 = OpLoad %float %30
%9 = OpFAdd %float %29 %31
OpReturnValue %9
OpFunctionEnd
)";
  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + before + nonEntryFuncs,
                                              predefs + after + nonEntryFuncs,
                                              false, true);
}

TEST_F(InlineTest, Decorated2) {
  // Same test as Simple with the difference
  // that the Result <id> of the outlined OpFunction
  // is decorated with RelaxedPrecision
  // Expected result is an equal decoration
  // of the created return variable
  //
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // float foo(vec4 bar)
  // {
  //     return bar.x + bar.y;
  // }
  //
  // void main()
  // {
  //     vec4 color = vec4(foo(BaseColor));
  //     gl_FragColor = color;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_ "foo(vf4;"
OpName %bar "bar"
OpName %color "color"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %foo_vf4_ RelaxedPrecision
)";

  const std::string before =
      R"(%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %float %_ptr_Function_v4float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %10
%21 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %BaseColor
OpStore %param %22
%23 = OpFunctionCall %float %foo_vf4_ %param
%24 = OpCompositeConstruct %v4float %23 %23 %23 %23
OpStore %color %24
%25 = OpLoad %v4float %color
OpStore %gl_FragColor %25
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpDecorate %32 RelaxedPrecision
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %float %_ptr_Function_v4float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %10
%21 = OpLabel
%32 = OpVariable %_ptr_Function_float Function
%color = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %BaseColor
OpStore %param %22
%33 = OpAccessChain %_ptr_Function_float %param %uint_0
%34 = OpLoad %float %33
%35 = OpAccessChain %_ptr_Function_float %param %uint_1
%36 = OpLoad %float %35
%37 = OpFAdd %float %34 %36
OpStore %32 %37
%23 = OpLoad %float %32
%24 = OpCompositeConstruct %v4float %23 %23 %23 %23
OpStore %color %24
%25 = OpLoad %v4float %color
OpStore %gl_FragColor %25
OpReturn
OpFunctionEnd
)";

  const std::string nonEntryFuncs =
      R"(%foo_vf4_ = OpFunction %float None %14
%bar = OpFunctionParameter %_ptr_Function_v4float
%26 = OpLabel
%27 = OpAccessChain %_ptr_Function_float %bar %uint_0
%28 = OpLoad %float %27
%29 = OpAccessChain %_ptr_Function_float %bar %uint_1
%30 = OpLoad %float %29
%31 = OpFAdd %float %28 %30
OpReturnValue %31
OpFunctionEnd
)";
  SinglePassRunAndCheck<InlineExhaustivePass>(predefs + before + nonEntryFuncs,
                                              predefs + after + nonEntryFuncs,
                                              false, true);
}

TEST_F(InlineTest, DeleteName) {
  // Test that the name of the result id of the call is deleted.
  const std::string before =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpName %main "main"
               OpName %main_entry "main_entry"
               OpName %foo_result "foo_result"
               OpName %void_fn "void_fn"
               OpName %foo "foo"
               OpName %foo_entry "foo_entry"
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %foo = OpFunction %void None %void_fn
  %foo_entry = OpLabel
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %void_fn
 %main_entry = OpLabel
 %foo_result = OpFunctionCall %void %foo
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
OpName %main_entry "main_entry"
OpName %void_fn "void_fn"
OpName %foo "foo"
OpName %foo_entry "foo_entry"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%foo_entry = OpLabel
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%main_entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, SetParent) {
  // Test that after inlining all basic blocks have the correct parent.
  const std::string text =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpName %main "main"
               OpName %main_entry "main_entry"
               OpName %foo_result "foo_result"
               OpName %void_fn "void_fn"
               OpName %foo "foo"
               OpName %foo_entry "foo_entry"
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %foo = OpFunction %void None %void_fn
  %foo_entry = OpLabel
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %void_fn
 %main_entry = OpLabel
 %foo_result = OpFunctionCall %void %foo
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  InlineExhaustivePass pass;
  pass.Run(context.get());

  for (Function& func : *context->module()) {
    for (BasicBlock& bb : func) {
      EXPECT_TRUE(bb.GetParent() == &func);
    }
  }
}

TEST_F(InlineTest, OpVariableWithInit) {
  // Check that there is a store that corresponds to the initializer.  This
  // test makes sure that is a store to the variable in the loop and before any
  // load.
  const std::string text = R"(
; CHECK: OpFunction
; CHECK-NOT: OpFunctionEnd
; CHECK: [[var:%\w+]] = OpVariable %_ptr_Function_float Function %float_0
; CHECK: OpLoopMerge [[outer_merge:%\w+]]
; CHECK-NOT: OpLoad %float [[var]]
; CHECK: OpStore [[var]] %float_0
; CHECK: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %o Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeFunction %float
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
          %o = OpVariable %_ptr_Output_float Output
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_int = OpTypePointer Input %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %o %float_0
               OpBranch %34
         %34 = OpLabel
         %39 = OpPhi %int %int_0 %5 %47 %37
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %41 = OpSLessThan %bool %39 %int_2
               OpBranchConditional %41 %35 %36
         %35 = OpLabel
         %42 = OpFunctionCall %float %foo_
         %43 = OpLoad %float %o
         %44 = OpFAdd %float %43 %42
               OpStore %o %44
               OpBranch %37
         %37 = OpLabel
         %47 = OpIAdd %int %39 %int_1
               OpBranch %34
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
       %foo_ = OpFunction %float None %7
          %9 = OpLabel
          %n = OpVariable %_ptr_Function_float Function %float_0
         %13 = OpLoad %float %n
         %15 = OpFOrdEqual %bool %13 %float_0
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
         %19 = OpLoad %float %n
         %20 = OpFAdd %float %19 %float_1
               OpStore %n %20
               OpBranch %17
         %17 = OpLabel
         %21 = OpLoad %float %n
               OpReturnValue %21
               OpFunctionEnd
)";

  SinglePassRunAndMatch<InlineExhaustivePass>(text, true);
}

TEST_F(InlineTest, DontInlineDirectlyRecursiveFunc) {
  // Test that the name of the result id of the call is deleted.
  const std::string test =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%15 = OpConstantNull %_struct_6
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %9
OpReturnValue %15
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InlineExhaustivePass>(test, test, false, true);
}

TEST_F(InlineTest, DontInlineInDirectlyRecursiveFunc) {
  // Test that the name of the result id of the call is deleted.
  const std::string test =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%15 = OpConstantNull %_struct_6
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %12
OpReturnValue %15
OpFunctionEnd
%12 = OpFunction %_struct_6 None %7
%13 = OpLabel
%14 = OpFunctionCall %_struct_6 %9
OpReturnValue %15
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InlineExhaustivePass>(test, test, false, true);
}

TEST_F(InlineTest, DontInlineFuncWithOpKillInContinue) {
  const std::string test =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 330
OpName %main "main"
OpName %kill_ "kill("
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %void None %3
%5 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %11 %12 None
OpBranch %13
%13 = OpLabel
OpBranchConditional %true %10 %11
%10 = OpLabel
OpBranch %12
%12 = OpLabel
%16 = OpFunctionCall %void %kill_
OpBranch %9
%11 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %void None %3
%7 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InlineExhaustivePass>(test, test, false, true);
}

TEST_F(InlineTest, InlineFuncWithOpKillNotInContinue) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 330
OpName %main "main"
OpName %kill_ "kill("
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpFunctionCall %void %kill_
OpReturn
OpFunctionEnd
%kill_ = OpFunction %void None %3
%7 = OpLabel
OpKill
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 330
OpName %main "main"
OpName %kill_ "kill("
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %void None %3
%5 = OpLabel
OpKill
%17 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %void None %3
%7 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugSimple) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 foo() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 4 1 %cu %main_name FlagIsProtected|FlagIsPrivate 4 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_lb = OpExtInst %void %ext DebugLexicalBlock %src 1 14 %dbg_foo
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
%foo_val = OpFunctionCall %v4float %foo
%color = OpLoad %v4float %in_var_COLOR
%result = OpFAdd %v4float %foo_val %color
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
%scope2 = OpExtInst %void %ext DebugScope %dbg_lb
OpReturnValue %v4f1
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 foo() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%18 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%20 = OpExtInst %void %1 DebugSource %5
%21 = OpExtInst %void %1 DebugCompilationUnit 1 4 %20 HLSL
%22 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%23 = OpExtInst %void %1 DebugTypeVector %22 4
%24 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23 %23
%25 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23
%26 = OpExtInst %void %1 DebugFunction %7 %24 %20 4 1 %21 %7 FlagIsProtected|FlagIsPrivate 4 %2
%27 = OpExtInst %void %1 DebugFunction %8 %25 %20 1 1 %21 %8 FlagIsProtected|FlagIsPrivate 1 %28
%29 = OpExtInst %void %1 DebugLexicalBlock %20 1 14 %27
%40 = OpExtInst %void %1 DebugInlinedAt 4 %26
%2 = OpFunction %void None %18
%30 = OpLabel
%39 = OpVariable %_ptr_Function_v4float Function
%41 = OpExtInst %void %1 DebugScope %27 %40
OpStore %39 %14
%32 = OpLoad %v4float %39
%42 = OpExtInst %void %1 DebugScope %26
%33 = OpLoad %v4float %3
%34 = OpFAdd %v4float %32 %33
OpStore %4 %34
%43 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%28 = OpFunction %v4float None %19
%36 = OpLabel
OpReturnValue %14
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugNested) {
  const std::string before = R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
%bar_name = OpString "bar"
%zoo_name = OpString "zoo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4f3 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 10 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_bar = OpExtInst %void %ext DebugFunction %bar_name %foo_ty %src 4 1 %cu %bar_name FlagIsProtected|FlagIsPrivate 4 %bar
%dbg_zoo = OpExtInst %void %ext DebugFunction %zoo_name %foo_ty %src 7 1 %cu %zoo_name FlagIsProtected|FlagIsPrivate 7 %zoo
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
%zoo_val = OpFunctionCall %v4float %zoo
%color = OpLoad %v4float %in_var_COLOR
%result = OpFAdd %v4float %zoo_val %color
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
OpReturnValue %v4f1
OpFunctionEnd
%bar = OpFunction %v4float None %v4f_fn_type
%scope2 = OpExtInst %void %ext DebugScope %dbg_bar
%bar_bb = OpLabel
%foo_val = OpFunctionCall %v4float %foo
%bar_ret = OpFAdd %v4float %foo_val %v4f2
OpReturnValue %bar_ret
OpFunctionEnd
%zoo = OpFunction %v4float None %v4f_fn_type
%scope3 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_bb = OpLabel
%bar_val = OpFunctionCall %v4float %bar
%zoo_ret = OpFAdd %v4float %bar_val %v4f3
OpReturnValue %zoo_ret
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
%9 = OpString "bar"
%10 = OpString "zoo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%19 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%20 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%24 = OpTypeFunction %void
%25 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 10 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugFunction %9 %31 %26 4 1 %27 %9 FlagIsProtected|FlagIsPrivate 4 %36
%37 = OpExtInst %void %1 DebugFunction %10 %31 %26 7 1 %27 %10 FlagIsProtected|FlagIsPrivate 7 %38
%58 = OpExtInst %void %1 DebugInlinedAt 10 %32
%62 = OpExtInst %void %1 DebugInlinedAt 7 %37 %58
%64 = OpExtInst %void %1 DebugInlinedAt 4 %35 %62
%2 = OpFunction %void None %24
%39 = OpLabel
%63 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%55 = OpVariable %_ptr_Function_v4float Function
%65 = OpExtInst %void %1 DebugScope %33 %64
OpStore %63 %18
%60 = OpLoad %v4float %63
%66 = OpExtInst %void %1 DebugScope %35 %62
%61 = OpFAdd %v4float %60 %19
OpStore %59 %61
%56 = OpLoad %v4float %59
%67 = OpExtInst %void %1 DebugScope %37 %58
%57 = OpFAdd %v4float %56 %20
OpStore %55 %57
%41 = OpLoad %v4float %55
%68 = OpExtInst %void %1 DebugScope %32
%42 = OpLoad %v4float %3
%43 = OpFAdd %v4float %41 %42
OpStore %4 %43
%69 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%34 = OpFunction %v4float None %25
%45 = OpLabel
OpReturnValue %18
OpFunctionEnd
%36 = OpFunction %v4float None %25
%47 = OpLabel
%70 = OpExtInst %void %1 DebugScope %35
%48 = OpFunctionCall %v4float %34
%49 = OpFAdd %v4float %48 %19
%71 = OpExtInst %void %1 DebugNoScope
OpReturnValue %49
OpFunctionEnd
%38 = OpFunction %v4float None %25
%51 = OpLabel
%72 = OpExtInst %void %1 DebugScope %37
%52 = OpFunctionCall %v4float %36
%53 = OpFAdd %v4float %52 %20
%73 = OpExtInst %void %1 DebugNoScope
OpReturnValue %53
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugSimpleHLSLPixelShader) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
return color;
}
"
%14 = OpString "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
return color;
}
"
%17 = OpString "float"
%21 = OpString "src.main"
%24 = OpString "color"
OpName %in_var_COLOR "in.var.COLOR"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %main "main"
OpName %param_var_color "param.var.color"
OpName %src_main "src.main"
OpName %color "color"
OpName %bb_entry "bb.entry"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%27 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%33 = OpTypeFunction %v4float %_ptr_Function_v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%13 = OpExtInst %void %1 DebugExpression
%15 = OpExtInst %void %1 DebugSource %5 %14
%16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
%18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
%19 = OpExtInst %void %1 DebugTypeVector %18 4
%20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %19 %19
%22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %src_main
%25 = OpExtInst %void %1 DebugLocalVariable %24 %19 %15 1 20 %22 FlagIsLocal 0
%26 = OpExtInst %void %1 DebugLexicalBlock %15 1 47 %22
%main = OpFunction %void None %27
%28 = OpLabel
%param_var_color = OpVariable %_ptr_Function_v4float Function
%31 = OpLoad %v4float %in_var_COLOR
OpStore %param_var_color %31
%32 = OpFunctionCall %v4float %src_main %param_var_color
OpStore %out_var_SV_TARGET %32
OpReturn
OpFunctionEnd
OpLine %5 1 1
%src_main = OpFunction %v4float None %33
%34 = OpExtInst %void %1 DebugScope %22
%color = OpFunctionParameter %_ptr_Function_v4float
%36 = OpExtInst %void %1 DebugDeclare %25 %color %13
%bb_entry = OpLabel
%38 = OpExtInst %void %1 DebugScope %26
OpLine %5 2 10
%39 = OpLoad %v4float %color
OpLine %5 2 3
OpReturnValue %39
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
return color;
}
"
%6 = OpString "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
return color;
}
"
%7 = OpString "float"
%8 = OpString "src.main"
%9 = OpString "color"
OpName %in_var_COLOR "in.var.COLOR"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %main "main"
OpName %param_var_color "param.var.color"
OpName %src_main "src.main"
OpName %color "color"
OpName %bb_entry "bb.entry"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%21 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%23 = OpTypeFunction %v4float %_ptr_Function_v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%24 = OpExtInst %void %1 DebugExpression
%25 = OpExtInst %void %1 DebugSource %5 %6
%26 = OpExtInst %void %1 DebugCompilationUnit 1 4 %25 HLSL
%27 = OpExtInst %void %1 DebugTypeBasic %7 %uint_32 Float
%28 = OpExtInst %void %1 DebugTypeVector %27 4
%29 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %28 %28
%30 = OpExtInst %void %1 DebugFunction %8 %29 %25 1 1 %26 %8 FlagIsProtected|FlagIsPrivate 1 %src_main
%31 = OpExtInst %void %1 DebugLocalVariable %9 %28 %25 1 20 %30 FlagIsLocal 0
%32 = OpExtInst %void %1 DebugLexicalBlock %25 1 47 %30
%main = OpFunction %void None %21
%33 = OpLabel
%41 = OpVariable %_ptr_Function_v4float Function
%param_var_color = OpVariable %_ptr_Function_v4float Function
%34 = OpLoad %v4float %in_var_COLOR
OpStore %param_var_color %34
%43 = OpExtInst %void %1 DebugScope %30
%40 = OpExtInst %void %1 DebugDeclare %31 %param_var_color %24
%44 = OpExtInst %void %1 DebugScope %32
OpLine %5 2 10
%42 = OpLoad %v4float %param_var_color
%45 = OpExtInst %void %1 DebugScope %30
OpStore %41 %42
%35 = OpLoad %v4float %41
%46 = OpExtInst %void %1 DebugNoScope
OpStore %out_var_SV_TARGET %35
OpReturn
OpFunctionEnd
OpLine %5 1 1
%src_main = OpFunction %v4float None %23
%47 = OpExtInst %void %1 DebugScope %30
%color = OpFunctionParameter %_ptr_Function_v4float
%37 = OpExtInst %void %1 DebugDeclare %31 %color %24
%48 = OpExtInst %void %1 DebugNoScope
%bb_entry = OpLabel
%49 = OpExtInst %void %1 DebugScope %32
OpLine %5 2 10
%39 = OpLoad %v4float %color
OpLine %5 2 3
%50 = OpExtInst %void %1 DebugNoScope
OpReturnValue %39
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugDeclareForCalleeFunctionParam) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%add_name = OpString "add"
%a_name = OpString "a"
%b_name = OpString "b"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%add_fn_type = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%add_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 5 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_add = OpExtInst %void %ext DebugFunction %add_name %add_ty %src 1 1 %cu %add_name FlagIsProtected|FlagIsPrivate 1 %add
%dbg_a = OpExtInst %void %ext DebugLocalVariable %a_name %dbg_v4f %src 1 13 %dbg_add FlagIsLocal 0
%dbg_b = OpExtInst %void %ext DebugLocalVariable %b_name %dbg_v4f %src 1 20 %dbg_add FlagIsLocal 1
%add_lb = OpExtInst %void %ext DebugLexicalBlock %src 1 23 %dbg_add
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%param_a = OpVariable %_ptr_Function_v4float Function
%param_b = OpVariable %_ptr_Function_v4float Function
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
OpStore %param_a %v4f1
OpStore %param_b %v4f2
%result = OpFunctionCall %v4float %add %param_a %param_b
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%add = OpFunction %v4float None %add_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_add
%a = OpFunctionParameter %_ptr_Function_v4float
%b = OpFunctionParameter %_ptr_Function_v4float
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_a %a %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_b %b %null_expr
%add_bb = OpLabel
%scope2 = OpExtInst %void %ext DebugScope %add_lb
%a_val = OpLoad %v4float %a
%b_val = OpLoad %v4float %b
%res = OpFAdd %v4float %a_val %b_val
OpReturnValue %res
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "add"
%9 = OpString "a"
%10 = OpString "b"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%18 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%22 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%24 = OpTypeFunction %void
%25 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%26 = OpExtInst %void %1 DebugExpression
%27 = OpExtInst %void %1 DebugSource %5
%28 = OpExtInst %void %1 DebugCompilationUnit 1 4 %27 HLSL
%29 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%30 = OpExtInst %void %1 DebugTypeVector %29 4
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %30 %30
%32 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %30 %30 %30
%33 = OpExtInst %void %1 DebugFunction %7 %31 %27 5 1 %28 %7 FlagIsProtected|FlagIsPrivate 10 %2
%34 = OpExtInst %void %1 DebugFunction %8 %32 %27 1 1 %28 %8 FlagIsProtected|FlagIsPrivate 1 %35
%36 = OpExtInst %void %1 DebugLocalVariable %9 %30 %27 1 13 %34 FlagIsLocal 0
%37 = OpExtInst %void %1 DebugLocalVariable %10 %30 %27 1 20 %34 FlagIsLocal 1
%38 = OpExtInst %void %1 DebugLexicalBlock %27 1 23 %34
%60 = OpExtInst %void %1 DebugInlinedAt 5 %33
%2 = OpFunction %void None %24
%39 = OpLabel
%56 = OpVariable %_ptr_Function_v4float Function
%40 = OpVariable %_ptr_Function_v4float Function
%41 = OpVariable %_ptr_Function_v4float Function
%61 = OpExtInst %void %1 DebugScope %33
OpStore %40 %17
OpStore %41 %18
%62 = OpExtInst %void %1 DebugScope %34 %60
%54 = OpExtInst %void %1 DebugDeclare %36 %40 %26
%55 = OpExtInst %void %1 DebugDeclare %37 %41 %26
%63 = OpExtInst %void %1 DebugScope %38 %60
%57 = OpLoad %v4float %40
%58 = OpLoad %v4float %41
%59 = OpFAdd %v4float %57 %58
%64 = OpExtInst %void %1 DebugScope %34 %60
OpStore %56 %59
%43 = OpLoad %v4float %56
%65 = OpExtInst %void %1 DebugScope %33
OpStore %4 %43
%66 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%35 = OpFunction %v4float None %22
%67 = OpExtInst %void %1 DebugScope %34
%45 = OpFunctionParameter %_ptr_Function_v4float
%46 = OpFunctionParameter %_ptr_Function_v4float
%47 = OpExtInst %void %1 DebugDeclare %36 %45 %26
%48 = OpExtInst %void %1 DebugDeclare %37 %46 %26
%68 = OpExtInst %void %1 DebugNoScope
%49 = OpLabel
%69 = OpExtInst %void %1 DebugScope %38
%51 = OpLoad %v4float %45
%52 = OpLoad %v4float %46
%53 = OpFAdd %v4float %51 %52
%70 = OpExtInst %void %1 DebugNoScope
OpReturnValue %53
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugDeclareForCalleeLocalVar) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%add_name = OpString "add"
%foo_name = OpString "foo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%add_fn_type = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%add_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 5 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_add = OpExtInst %void %ext DebugFunction %add_name %add_ty %src 1 1 %cu %add_name FlagIsProtected|FlagIsPrivate 1 %add
%dbg_foo = OpExtInst %void %ext DebugLocalVariable %foo_name %dbg_v4f %src 2 2 %dbg_add FlagIsLocal
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%param_a = OpVariable %_ptr_Function_v4float Function
%param_b = OpVariable %_ptr_Function_v4float Function
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
OpStore %param_a %v4f1
OpStore %param_b %v4f2
%result = OpFunctionCall %v4float %add %param_a %param_b
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%add = OpFunction %v4float None %add_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_add
%a = OpFunctionParameter %_ptr_Function_v4float
%b = OpFunctionParameter %_ptr_Function_v4float
%add_bb = OpLabel
%foo = OpVariable %_ptr_Function_v4float Function
%a_val = OpLoad %v4float %a
%b_val = OpLoad %v4float %b
%res = OpFAdd %v4float %a_val %b_val
OpStore %foo %res
%decl = OpExtInst %void %ext DebugDeclare %dbg_foo %foo %null_expr
%foo_val = OpLoad %v4float %foo
OpReturnValue %foo_val
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "add"
%9 = OpString "foo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%16 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%17 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%21 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%23 = OpTypeFunction %void
%24 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%25 = OpExtInst %void %1 DebugExpression
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29 %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 5 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugLocalVariable %9 %29 %26 2 2 %33 FlagIsLocal
%58 = OpExtInst %void %1 DebugInlinedAt 5 %32
%2 = OpFunction %void None %23
%36 = OpLabel
%59 = OpExtInst %void %1 DebugScope %33 %58
%51 = OpVariable %_ptr_Function_v4float Function
%60 = OpExtInst %void %1 DebugNoScope
%52 = OpVariable %_ptr_Function_v4float Function
%37 = OpVariable %_ptr_Function_v4float Function
%38 = OpVariable %_ptr_Function_v4float Function
%61 = OpExtInst %void %1 DebugScope %32
OpStore %37 %16
OpStore %38 %17
%62 = OpExtInst %void %1 DebugScope %33 %58
%53 = OpLoad %v4float %37
%54 = OpLoad %v4float %38
%55 = OpFAdd %v4float %53 %54
OpStore %51 %55
%56 = OpExtInst %void %1 DebugDeclare %35 %51 %25
%57 = OpLoad %v4float %51
OpStore %52 %57
%40 = OpLoad %v4float %52
%63 = OpExtInst %void %1 DebugScope %32
OpStore %4 %40
%64 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%34 = OpFunction %v4float None %21
%65 = OpExtInst %void %1 DebugScope %33
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpFunctionParameter %_ptr_Function_v4float
%66 = OpExtInst %void %1 DebugNoScope
%44 = OpLabel
%67 = OpExtInst %void %1 DebugScope %33
%45 = OpVariable %_ptr_Function_v4float Function
%46 = OpLoad %v4float %42
%47 = OpLoad %v4float %43
%48 = OpFAdd %v4float %46 %47
OpStore %45 %48
%49 = OpExtInst %void %1 DebugDeclare %35 %45 %25
%50 = OpLoad %v4float %45
%68 = OpExtInst %void %1 DebugNoScope
OpReturnValue %50
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugValueForFunctionCallReturn) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%add_name = OpString "add"
%result_name = OpString "result"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%add_fn_type = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%add_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 5 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_add = OpExtInst %void %ext DebugFunction %add_name %add_ty %src 1 1 %cu %add_name FlagIsProtected|FlagIsPrivate 1 %add
%dbg_result = OpExtInst %void %ext DebugLocalVariable %result_name %dbg_v4f %src 6 2 %dbg_main FlagIsLocal
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%param_a = OpVariable %_ptr_Function_v4float Function
%param_b = OpVariable %_ptr_Function_v4float Function
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
OpStore %param_a %v4f1
OpStore %param_b %v4f2
%result = OpFunctionCall %v4float %add %param_a %param_b
%value = OpExtInst %void %ext DebugValue %dbg_result %result %null_expr
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%add = OpFunction %v4float None %add_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_add
%a = OpFunctionParameter %_ptr_Function_v4float
%b = OpFunctionParameter %_ptr_Function_v4float
%add_bb = OpLabel
%a_val = OpLoad %v4float %a
%b_val = OpLoad %v4float %b
%res = OpFAdd %v4float %a_val %b_val
OpReturnValue %res
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "add"
%9 = OpString "result"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v4float = OpTypeVector %float 4
%16 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%17 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%21 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%23 = OpTypeFunction %void
%24 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%25 = OpExtInst %void %1 DebugExpression
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29 %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 5 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugLocalVariable %9 %29 %26 6 2 %32 FlagIsLocal
%53 = OpExtInst %void %1 DebugInlinedAt 5 %32
%2 = OpFunction %void None %23
%36 = OpLabel
%49 = OpVariable %_ptr_Function_v4float Function
%37 = OpVariable %_ptr_Function_v4float Function
%38 = OpVariable %_ptr_Function_v4float Function
%54 = OpExtInst %void %1 DebugScope %32
OpStore %37 %16
OpStore %38 %17
%55 = OpExtInst %void %1 DebugScope %33 %53
%50 = OpLoad %v4float %37
%51 = OpLoad %v4float %38
%52 = OpFAdd %v4float %50 %51
OpStore %49 %52
%40 = OpLoad %v4float %49
%56 = OpExtInst %void %1 DebugScope %32
%41 = OpExtInst %void %1 DebugValue %35 %40 %25
OpStore %4 %40
%57 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%34 = OpFunction %v4float None %21
%58 = OpExtInst %void %1 DebugScope %33
%43 = OpFunctionParameter %_ptr_Function_v4float
%44 = OpFunctionParameter %_ptr_Function_v4float
%59 = OpExtInst %void %1 DebugNoScope
%45 = OpLabel
%60 = OpExtInst %void %1 DebugScope %33
%46 = OpLoad %v4float %43
%47 = OpLoad %v4float %44
%48 = OpFAdd %v4float %46 %47
%61 = OpExtInst %void %1 DebugNoScope
OpReturnValue %48
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, NestedWithAnExistingDebugInlinedAt) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
%bar_name = OpString "bar"
%zoo_name = OpString "zoo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4f3 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 10 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_bar = OpExtInst %void %ext DebugFunction %bar_name %foo_ty %src 4 1 %cu %bar_name FlagIsProtected|FlagIsPrivate 4 %bar
%dbg_zoo = OpExtInst %void %ext DebugFunction %zoo_name %foo_ty %src 7 1 %cu %zoo_name FlagIsProtected|FlagIsPrivate 7 %zoo
%inlined_to_zoo = OpExtInst %void %ext DebugInlinedAt 7 %dbg_zoo
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
%zoo_val = OpFunctionCall %v4float %zoo
%color = OpLoad %v4float %in_var_COLOR
%result = OpFAdd %v4float %zoo_val %color
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
OpReturnValue %v4f1
OpFunctionEnd
%zoo = OpFunction %v4float None %v4f_fn_type
%scope3 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_bb = OpLabel
%scope2 = OpExtInst %void %ext DebugScope %dbg_bar %inlined_to_zoo
%foo_val = OpFunctionCall %v4float %foo
%bar_val = OpFAdd %v4float %foo_val %v4f2
%scope4 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_ret = OpFAdd %v4float %bar_val %v4f3
OpReturnValue %zoo_ret
OpFunctionEnd
%bar = OpFunction %v4float None %v4f_fn_type
%scope5 = OpExtInst %void %ext DebugScope %dbg_bar
%bar_bb = OpLabel
%foo_val0 = OpFunctionCall %v4float %foo
%bar_ret = OpFAdd %v4float %foo_val0 %v4f2
OpReturnValue %bar_ret
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
%9 = OpString "bar"
%10 = OpString "zoo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%19 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%20 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%24 = OpTypeFunction %void
%25 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 10 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugFunction %9 %31 %26 4 1 %27 %9 FlagIsProtected|FlagIsPrivate 4 %36
%37 = OpExtInst %void %1 DebugFunction %10 %31 %26 7 1 %27 %10 FlagIsProtected|FlagIsPrivate 7 %38
%39 = OpExtInst %void %1 DebugInlinedAt 7 %37
%63 = OpExtInst %void %1 DebugInlinedAt 10 %32
%64 = OpExtInst %void %1 DebugInlinedAt 7 %37 %63
%66 = OpExtInst %void %1 DebugInlinedAt 4 %35 %64
%2 = OpFunction %void None %24
%40 = OpLabel
%65 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%67 = OpExtInst %void %1 DebugScope %33 %66
OpStore %65 %18
%60 = OpLoad %v4float %65
%68 = OpExtInst %void %1 DebugScope %35 %64
%61 = OpFAdd %v4float %60 %19
%69 = OpExtInst %void %1 DebugScope %37 %63
%62 = OpFAdd %v4float %61 %20
OpStore %59 %62
%42 = OpLoad %v4float %59
%70 = OpExtInst %void %1 DebugScope %32
%43 = OpLoad %v4float %3
%44 = OpFAdd %v4float %42 %43
OpStore %4 %44
%71 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%34 = OpFunction %v4float None %25
%46 = OpLabel
OpReturnValue %18
OpFunctionEnd
%38 = OpFunction %v4float None %25
%48 = OpLabel
%72 = OpExtInst %void %1 DebugScope %35 %39
%50 = OpFunctionCall %v4float %34
%51 = OpFAdd %v4float %50 %19
%73 = OpExtInst %void %1 DebugScope %37
%53 = OpFAdd %v4float %51 %20
%74 = OpExtInst %void %1 DebugNoScope
OpReturnValue %53
OpFunctionEnd
%36 = OpFunction %v4float None %25
%55 = OpLabel
%75 = OpExtInst %void %1 DebugScope %35
%56 = OpFunctionCall %v4float %34
%57 = OpFAdd %v4float %56 %19
%76 = OpExtInst %void %1 DebugNoScope
OpReturnValue %57
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, NestedWithTwoExistingDebugInlinedAt) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
%bar_name = OpString "bar"
%zoo_name = OpString "zoo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4f3 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 10 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_bar = OpExtInst %void %ext DebugFunction %bar_name %foo_ty %src 4 1 %cu %bar_name FlagIsProtected|FlagIsPrivate 4 %bar
%dbg_zoo = OpExtInst %void %ext DebugFunction %zoo_name %foo_ty %src 7 1 %cu %zoo_name FlagIsProtected|FlagIsPrivate 7 %zoo
%inlined_to_zoo = OpExtInst %void %ext DebugInlinedAt 7 %dbg_zoo
%inlined_to_bar = OpExtInst %void %ext DebugInlinedAt 4 %dbg_bar %inlined_to_zoo
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
%zoo_val = OpFunctionCall %v4float %zoo
%color = OpLoad %v4float %in_var_COLOR
%result = OpFAdd %v4float %zoo_val %color
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%zoo = OpFunction %v4float None %v4f_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_bb = OpLabel
%scope2 = OpExtInst %void %ext DebugScope %dbg_foo %inlined_to_bar
%foo_val = OpLoad %v4float %in_var_COLOR
%scope3 = OpExtInst %void %ext DebugScope %dbg_bar %inlined_to_zoo
%bar_val = OpFAdd %v4float %foo_val %v4f2
%scope4 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_ret = OpFAdd %v4float %bar_val %v4f3
OpReturnValue %zoo_ret
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%scope5 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
%result0 = OpLoad %v4float %in_var_COLOR
OpReturnValue %result0
OpFunctionEnd
%bar = OpFunction %v4float None %v4f_fn_type
%scope6 = OpExtInst %void %ext DebugScope %dbg_bar
%bar_bb = OpLabel
%foo_val0 = OpFunctionCall %v4float %foo
%bar_ret = OpFAdd %v4float %foo_val0 %v4f2
OpReturnValue %bar_ret
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
%9 = OpString "bar"
%10 = OpString "zoo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%19 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%20 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%24 = OpTypeFunction %void
%25 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 10 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugFunction %9 %31 %26 4 1 %27 %9 FlagIsProtected|FlagIsPrivate 4 %36
%37 = OpExtInst %void %1 DebugFunction %10 %31 %26 7 1 %27 %10 FlagIsProtected|FlagIsPrivate 7 %38
%39 = OpExtInst %void %1 DebugInlinedAt 7 %37
%40 = OpExtInst %void %1 DebugInlinedAt 4 %35 %39
%66 = OpExtInst %void %1 DebugInlinedAt 10 %32
%68 = OpExtInst %void %1 DebugInlinedAt 7 %37 %66
%67 = OpExtInst %void %1 DebugInlinedAt 4 %35 %68
%2 = OpFunction %void None %24
%41 = OpLabel
%62 = OpVariable %_ptr_Function_v4float Function
%69 = OpExtInst %void %1 DebugScope %33 %67
%63 = OpLoad %v4float %3
%70 = OpExtInst %void %1 DebugScope %35 %68
%64 = OpFAdd %v4float %63 %19
%71 = OpExtInst %void %1 DebugScope %37 %66
%65 = OpFAdd %v4float %64 %20
OpStore %62 %65
%43 = OpLoad %v4float %62
%72 = OpExtInst %void %1 DebugScope %32
%44 = OpLoad %v4float %3
%45 = OpFAdd %v4float %43 %44
OpStore %4 %45
%73 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%38 = OpFunction %v4float None %25
%47 = OpLabel
%74 = OpExtInst %void %1 DebugScope %33 %40
%49 = OpLoad %v4float %3
%75 = OpExtInst %void %1 DebugScope %35 %39
%51 = OpFAdd %v4float %49 %19
%76 = OpExtInst %void %1 DebugScope %37
%53 = OpFAdd %v4float %51 %20
%77 = OpExtInst %void %1 DebugNoScope
OpReturnValue %53
OpFunctionEnd
%34 = OpFunction %v4float None %25
%55 = OpLabel
%78 = OpExtInst %void %1 DebugScope %33
%56 = OpLoad %v4float %3
%79 = OpExtInst %void %1 DebugNoScope
OpReturnValue %56
OpFunctionEnd
%36 = OpFunction %v4float None %25
%58 = OpLabel
%80 = OpExtInst %void %1 DebugScope %35
%59 = OpFunctionCall %v4float %34
%60 = OpFAdd %v4float %59 %19
%81 = OpExtInst %void %1 DebugNoScope
OpReturnValue %60
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugMultipleInlining) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 foo() {
  return 1;
}
float4 bar() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + bar() + color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
%bar_name = OpString "bar"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 7 1 %cu %main_name FlagIsProtected|FlagIsPrivate 7 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_bar = OpExtInst %void %ext DebugFunction %bar_name %foo_ty %src 4 1 %cu %bar_name FlagIsProtected|FlagIsPrivate 4 %bar
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%scope0 = OpExtInst %void %ext DebugScope %dbg_main
%foo_val = OpFunctionCall %v4float %foo
%color = OpLoad %v4float %in_var_COLOR
%add = OpFAdd %v4float %foo_val %color
%bar_val = OpFunctionCall %v4float %bar
%result = OpFAdd %v4float %bar_val %add
OpStore %out_var_SV_TARGET %result
OpReturn
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%scope1 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
OpReturnValue %v4f1
OpFunctionEnd
%bar = OpFunction %v4float None %v4f_fn_type
%scope2 = OpExtInst %void %ext DebugScope %dbg_bar
%bar_bb = OpLabel
OpReturnValue %v4f1
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 foo() {
  return 1;
}
float4 bar() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + bar() + color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
%9 = OpString "bar"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%15 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%19 = OpTypeFunction %void
%20 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%21 = OpExtInst %void %1 DebugSource %5
%22 = OpExtInst %void %1 DebugCompilationUnit 1 4 %21 HLSL
%23 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%24 = OpExtInst %void %1 DebugTypeVector %23 4
%25 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %24 %24
%26 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %24
%27 = OpExtInst %void %1 DebugFunction %7 %25 %21 7 1 %22 %7 FlagIsProtected|FlagIsPrivate 7 %2
%28 = OpExtInst %void %1 DebugFunction %8 %26 %21 1 1 %22 %8 FlagIsProtected|FlagIsPrivate 1 %29
%30 = OpExtInst %void %1 DebugFunction %9 %26 %21 4 1 %22 %9 FlagIsProtected|FlagIsPrivate 4 %31
%45 = OpExtInst %void %1 DebugInlinedAt 7 %27
%47 = OpExtInst %void %1 DebugInlinedAt 7 %27
%2 = OpFunction %void None %19
%32 = OpLabel
%46 = OpVariable %_ptr_Function_v4float Function
%44 = OpVariable %_ptr_Function_v4float Function
%48 = OpExtInst %void %1 DebugScope %28 %45
OpStore %44 %15
%34 = OpLoad %v4float %44
%49 = OpExtInst %void %1 DebugScope %27
%35 = OpLoad %v4float %3
%36 = OpFAdd %v4float %34 %35
%50 = OpExtInst %void %1 DebugScope %30 %47
OpStore %46 %15
%37 = OpLoad %v4float %46
%51 = OpExtInst %void %1 DebugScope %27
%38 = OpFAdd %v4float %37 %36
OpStore %4 %38
%52 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%29 = OpFunction %v4float None %20
%40 = OpLabel
OpReturnValue %15
OpFunctionEnd
%31 = OpFunction %v4float None %20
%42 = OpLabel
OpReturnValue %15
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

TEST_F(InlineTest, DebugManyNested) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "ps.hlsl"
OpSource HLSL 600 %file_name "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%float_name = OpString "float"
%main_name = OpString "main"
%foo_name = OpString "foo"
%bar_name = OpString "bar"
%zoo_name = OpString "zoo"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%v4f1 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4f2 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4f3 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%void_fn_type = OpTypeFunction %void
%v4f_fn_type = OpTypeFunction %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_f = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_f 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%foo_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 10 1 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_foo = OpExtInst %void %ext DebugFunction %foo_name %foo_ty %src 1 1 %cu %foo_name FlagIsProtected|FlagIsPrivate 1 %foo
%dbg_bar = OpExtInst %void %ext DebugFunction %bar_name %foo_ty %src 4 1 %cu %bar_name FlagIsProtected|FlagIsPrivate 4 %bar
%dbg_zoo = OpExtInst %void %ext DebugFunction %zoo_name %foo_ty %src 7 1 %cu %zoo_name FlagIsProtected|FlagIsPrivate 7 %zoo
%inlined_to_zoo = OpExtInst %void %ext DebugInlinedAt 7 %dbg_zoo
%inlined_to_bar_and_zoo = OpExtInst %void %ext DebugInlinedAt 4 %dbg_bar %inlined_to_zoo
%inlined_to_bar = OpExtInst %void %ext DebugInlinedAt 4 %dbg_bar
%main = OpFunction %void None %void_fn_type
%main_bb = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%zoo_val = OpFunctionCall %v4float %zoo
%color = OpLoad %v4float %in_var_COLOR
%r0 = OpFAdd %v4float %zoo_val %color
%foo_val = OpFunctionCall %v4float %foo
%r1 = OpFAdd %v4float %foo_val %r0
%bar_val = OpFunctionCall %v4float %bar
%r2 = OpFAdd %v4float %bar_val %r0
OpStore %out_var_SV_TARGET %r2
OpReturn
OpFunctionEnd
%foo = OpFunction %v4float None %v4f_fn_type
%s1 = OpExtInst %void %ext DebugScope %dbg_foo
%foo_bb = OpLabel
%foo_ret = OpLoad %v4float %in_var_COLOR
OpReturnValue %foo_ret
OpFunctionEnd
%zoo = OpFunction %v4float None %v4f_fn_type
%s2 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_bb = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_bar %inlined_to_zoo
%foo0 = OpFunctionCall %v4float %foo
%bar0 = OpFAdd %v4float %foo0 %v4f2
%s4 = OpExtInst %void %ext DebugScope %dbg_foo %inlined_to_zoo
%foo1 = OpLoad %v4float %in_var_COLOR
%s5 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo0 = OpFAdd %v4float %bar0 %foo1
%s6 = OpExtInst %void %ext DebugScope %dbg_foo %inlined_to_bar_and_zoo
%foo2 = OpLoad %v4float %in_var_COLOR
%s7 = OpExtInst %void %ext DebugScope %dbg_bar %inlined_to_zoo
%bar1 = OpFAdd %v4float %foo2 %v4f2
%s8 = OpExtInst %void %ext DebugScope %dbg_zoo
%zoo_ret = OpFAdd %v4float %zoo0 %bar1
OpReturnValue %zoo_ret
OpFunctionEnd
%bar = OpFunction %v4float None %v4f_fn_type
%bar_bb = OpLabel
%s9 = OpExtInst %void %ext DebugScope %dbg_foo %inlined_to_bar
%foo3 = OpLoad %v4float %in_var_COLOR
%s10 = OpExtInst %void %ext DebugScope %dbg_bar
%bar_ret = OpFAdd %v4float %foo3 %v4f2
OpReturnValue %bar_ret
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "#line 1 \"ps.hlsl\"
float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
%9 = OpString "bar"
%10 = OpString "zoo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v4float = OpTypeVector %float 4
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%19 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%20 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%24 = OpTypeFunction %void
%25 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
%26 = OpExtInst %void %1 DebugSource %5
%27 = OpExtInst %void %1 DebugCompilationUnit 1 4 %26 HLSL
%28 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%29 = OpExtInst %void %1 DebugTypeVector %28 4
%30 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29 %29
%31 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %29
%32 = OpExtInst %void %1 DebugFunction %7 %30 %26 10 1 %27 %7 FlagIsProtected|FlagIsPrivate 10 %2
%33 = OpExtInst %void %1 DebugFunction %8 %31 %26 1 1 %27 %8 FlagIsProtected|FlagIsPrivate 1 %34
%35 = OpExtInst %void %1 DebugFunction %9 %31 %26 4 1 %27 %9 FlagIsProtected|FlagIsPrivate 4 %36
%37 = OpExtInst %void %1 DebugFunction %10 %31 %26 7 1 %27 %10 FlagIsProtected|FlagIsPrivate 7 %38
%39 = OpExtInst %void %1 DebugInlinedAt 7 %37
%40 = OpExtInst %void %1 DebugInlinedAt 4 %35 %39
%41 = OpExtInst %void %1 DebugInlinedAt 4 %35
%83 = OpExtInst %void %1 DebugInlinedAt 10 %32
%84 = OpExtInst %void %1 DebugInlinedAt 7 %37 %83
%85 = OpExtInst %void %1 DebugInlinedAt 4 %35 %84
%88 = OpExtInst %void %1 DebugInlinedAt 4 %35 %84
%91 = OpExtInst %void %1 DebugInlinedAt 10 %32
%95 = OpExtInst %void %1 DebugInlinedAt 10 %32
%96 = OpExtInst %void %1 DebugInlinedAt 4 %35 %95
%2 = OpFunction %void None %24
%42 = OpLabel
%92 = OpVariable %_ptr_Function_v4float Function
%89 = OpVariable %_ptr_Function_v4float Function
%86 = OpVariable %_ptr_Function_v4float Function
%75 = OpVariable %_ptr_Function_v4float Function
%97 = OpExtInst %void %1 DebugScope %33 %88
%87 = OpLoad %v4float %3
OpStore %86 %87
%76 = OpLoad %v4float %86
%98 = OpExtInst %void %1 DebugScope %35 %84
%77 = OpFAdd %v4float %76 %19
%99 = OpExtInst %void %1 DebugScope %33 %84
%78 = OpLoad %v4float %3
%100 = OpExtInst %void %1 DebugScope %37 %83
%79 = OpFAdd %v4float %77 %78
%101 = OpExtInst %void %1 DebugScope %33 %85
%80 = OpLoad %v4float %3
%102 = OpExtInst %void %1 DebugScope %35 %84
%81 = OpFAdd %v4float %80 %19
%103 = OpExtInst %void %1 DebugScope %37 %83
%82 = OpFAdd %v4float %79 %81
OpStore %75 %82
%44 = OpLoad %v4float %75
%104 = OpExtInst %void %1 DebugScope %32
%45 = OpLoad %v4float %3
%46 = OpFAdd %v4float %44 %45
%105 = OpExtInst %void %1 DebugScope %33 %91
%90 = OpLoad %v4float %3
OpStore %89 %90
%47 = OpLoad %v4float %89
%106 = OpExtInst %void %1 DebugScope %32
%48 = OpFAdd %v4float %47 %46
%107 = OpExtInst %void %1 DebugScope %33 %96
%93 = OpLoad %v4float %3
%108 = OpExtInst %void %1 DebugScope %35 %95
%94 = OpFAdd %v4float %93 %19
OpStore %92 %94
%49 = OpLoad %v4float %92
%109 = OpExtInst %void %1 DebugScope %32
%50 = OpFAdd %v4float %49 %46
OpStore %4 %50
%110 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%34 = OpFunction %v4float None %25
%52 = OpLabel
%111 = OpExtInst %void %1 DebugScope %33
%53 = OpLoad %v4float %3
%112 = OpExtInst %void %1 DebugNoScope
OpReturnValue %53
OpFunctionEnd
%38 = OpFunction %v4float None %25
%55 = OpLabel
%113 = OpExtInst %void %1 DebugScope %35 %39
%57 = OpFunctionCall %v4float %34
%58 = OpFAdd %v4float %57 %19
%114 = OpExtInst %void %1 DebugScope %33 %39
%60 = OpLoad %v4float %3
%115 = OpExtInst %void %1 DebugScope %37
%62 = OpFAdd %v4float %58 %60
%116 = OpExtInst %void %1 DebugScope %33 %40
%64 = OpLoad %v4float %3
%117 = OpExtInst %void %1 DebugScope %35 %39
%66 = OpFAdd %v4float %64 %19
%118 = OpExtInst %void %1 DebugScope %37
%68 = OpFAdd %v4float %62 %66
%119 = OpExtInst %void %1 DebugNoScope
OpReturnValue %68
OpFunctionEnd
%36 = OpFunction %v4float None %25
%69 = OpLabel
%120 = OpExtInst %void %1 DebugScope %33 %41
%71 = OpLoad %v4float %3
%121 = OpExtInst %void %1 DebugScope %35
%73 = OpFAdd %v4float %71 %19
%122 = OpExtInst %void %1 DebugNoScope
OpReturnValue %73
OpFunctionEnd
)";

  SinglePassRunAndCheck<InlineExhaustivePass>(before, after, false, true);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    Empty modules
//    Modules without function definitions
//    Modules in which all functions do not call other functions
//    Caller and callee both accessing the same global variable
//    Functions with OpLine & OpNoLine
//    Others?

// TODO(dneto): Test suggestions from code review
// https://github.com/KhronosGroup/SPIRV-Tools/pull/534
//
//    Callee function returns a value generated outside the callee,
//      e.g. a constant value. This might exercise some logic not yet
//      exercised by the current tests: the false branch in the "if"
//      inside the SpvOpReturnValue case in InlinePass::GenInlineCode?
//    SampledImage before function call, but callee is only single block.
//      Then the SampledImage instruction is not cloned. Documents existing
//      behaviour.
//    SampledImage after function call. It is not cloned or changed.

}  // namespace
}  // namespace opt
}  // namespace spvtools
