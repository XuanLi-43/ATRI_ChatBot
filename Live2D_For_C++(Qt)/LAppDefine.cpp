/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppDefine.hpp"
#include <CubismFramework.hpp>

namespace LAppDefine {

    using namespace Csm;

    // live2d角色缩放比例
    const csmFloat32 ViewScale = 0.9f;
    const csmFloat32 ViewMaxScale = 1.0f;
    const csmFloat32 ViewMinScale = 0.8f;

    const csmFloat32 ViewLogicalLeft = -1.0f;
    const csmFloat32 ViewLogicalRight = 1.0f;
    const csmFloat32 ViewLogicalBottom = -1.0f;
    const csmFloat32 ViewLogicalTop = -1.0f;

    const csmFloat32 ViewLogicalMaxLeft = -2.0f;
    const csmFloat32 ViewLogicalMaxRight = 2.0f;
    const csmFloat32 ViewLogicalMaxBottom = -3.0f;
    const csmFloat32 ViewLogicalMaxTop = 2.0f;

    // 相对路径
    const csmChar* ResourcesPath = "Resources/";

    // 背景图片
    const csmChar* BackImageName = "background.png";
    ////// 不要动, 删了会出现空指针异常
    //const csmChar* GearImageName = "dont_move1.png";
    ////// 不要动
    //const csmChar* PowerImageName = "dont_move2.png";

    
    const csmChar* ShaderPath = "SampleShaders/";
    // 頂点シェーダー
    const csmChar* VertShaderName = "VertSprite.vert";
    // フラグメントシェーダー
    const csmChar* FragShaderName = "FragSprite.frag";

    // モデル定義------------------------------------------
    // 与外部定义文件(json)保持一致
    const csmChar* MotionGroupIdle = "Idle"; // 空闲状态
    const csmChar* MotionGroupTapBody = "TapBody"; // 点击身体时

    // 外部定义文件(json)保持一致 c
    const csmChar* HitAreaNameHead = "Head";
    const csmChar* HitAreaNameBody = "Body";

    // モーションの優先度定数
    const csmInt32 PriorityNone = 0;
    const csmInt32 PriorityIdle = 1;
    const csmInt32 PriorityNormal = 2;
    const csmInt32 PriorityForce = 3;

    // MOC3の整合性検証オプション
    const csmBool MocConsistencyValidationEnable = true;
    // motion3.jsonの整合性検証オプション
    const csmBool MotionConsistencyValidationEnable = true;

    // デバッグ用ログの表示オプション
    const csmBool DebugLogEnable = true;
    const csmBool DebugTouchLogEnable = false;

    // Frameworkから出力するログのレベル設定
    const CubismFramework::Option::LogLevel CubismLoggingLevel = CubismFramework::Option::LogLevel_Verbose;

    // デフォルトのレンダーターゲットサイズ
    const csmInt32 RenderTargetWidth = 1900;
    const csmInt32 RenderTargetHeight = 1000;
}
