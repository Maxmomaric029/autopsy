#pragma once
#include <cstdint>
#include "../../core/offsets_manager.h"

namespace offset {

// init() - populate runtime offsets from OffsetsManager (fallback to defaults)
inline void init();

    inline uintptr_t air_AirDensity = 0x18;
    inline uintptr_t air_GlobalWind = 0x3c;

    namespace air {
         inline uintptr_t AirDensity = 0x18;
         inline uintptr_t GlobalWind = 0x3c;
    }

    namespace animation {
        inline uintptr_t Animation = 0xd0;
        inline uintptr_t animator = 0x118;
        inline uintptr_t IsPlaying = 0xa10;
        inline uintptr_t Looped = 0xf5;
        inline uintptr_t Speed = 0xe4;
        inline uintptr_t TimePosition = 0xe8;
    }

    namespace animator {
        inline uintptr_t ActiveAnimations = 0x868;
    }

    namespace atmosphere {
        inline uintptr_t tocolor = 0xd0;
        inline uintptr_t Decay = 0xdc;
        inline uintptr_t Density = 0xe8;
        inline uintptr_t Glare = 0xec;
        inline uintptr_t Haze = 0xf0;
        inline uintptr_t Offset = 0xf4;
    }

    namespace attachment {
        inline uintptr_t Position = 0xdc;
    }

    namespace basepart {
        inline uintptr_t CastShadow = 0xf5;
        inline uintptr_t Color3 = 0x194;
        inline uintptr_t Locked = 0xf6;
        inline uintptr_t Massless = 0xf7;
        inline uintptr_t primitive = 0x148;
        inline uintptr_t Reflectance = 0xec;
        inline uintptr_t Shape = 0x1b1;
        inline uintptr_t Transparency = 0xf0;
    }

    namespace beam {
        inline uintptr_t Attachment0 = 0x170;
        inline uintptr_t Attachment1 = 0x180;
        inline uintptr_t Brightness = 0x190;
        inline uintptr_t CurveSize0 = 0x194;
        inline uintptr_t CurveSize1 = 0x198;
        inline uintptr_t LightEmission = 0x19c;
        inline uintptr_t LightInfluence = 0x1a0;
        inline uintptr_t Texture = 0x150;
        inline uintptr_t TextureLength = 0x1ac;
        inline uintptr_t TextureSpeed = 0x1b4;
        inline uintptr_t Width0 = 0x1b8;
        inline uintptr_t Width1 = 0x1bc;
        inline uintptr_t ZOffset = 0x1c0;
    }

    namespace bloom {
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t Intensity = 0xd0;
        inline uintptr_t Size = 0xd4;
        inline uintptr_t Threshold = 0xd8;
    }

    namespace blur {
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t Size = 0xd0;
    }

    namespace bytecode {
        inline uintptr_t Pointer = 0x10;
        inline uintptr_t Size = 0x20;
    }

    namespace camera {
        inline uintptr_t CameraSubject = 0xe8;
        inline uintptr_t CameraType = 0x158;
        inline uintptr_t FieldOfView = 0x160;
        inline uintptr_t ImagePlaneDepth = 0x2f0;
        inline uintptr_t Position = 0x11c;
        inline uintptr_t Rotation = 0xf8;
        inline uintptr_t Viewport = 0x2ac;
        inline uintptr_t ViewportSize = 0x2e8;
    }

    namespace charmesh {
        inline uintptr_t BaseTextureId = 0xe0;
        inline uintptr_t BodyPart = 0x160;
        inline uintptr_t MeshId = 0x110;
        inline uintptr_t OverlayTextureId = 0x140;
    }

    namespace click {
        inline uintptr_t MaxActivationDistance = 0x100;
        inline uintptr_t MouseIcon = 0xe0;
    }

    namespace clothing {
        inline uintptr_t Color3 = 0x128;
        inline uintptr_t Template = 0x108;
    }

    namespace correction {
        inline uintptr_t Brightness = 0xdc;
        inline uintptr_t Contrast = 0xe0;
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t TintColor = 0xd0;
    }

    namespace grading {
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t TonemapperPreset = 0xd0;
    }

    namespace datamodel {
        inline uintptr_t CreatorId = 0x190;
        inline uintptr_t GameId = 0x198;
        inline uintptr_t GameLoaded = 0x638;
        inline uintptr_t JobId = 0x138;
        inline uintptr_t PlaceId = 0x1a0;
        inline uintptr_t PlaceVersion = 0x1bc;
        inline uintptr_t PrimitiveCount = 0x480;
        inline uintptr_t scriptcontext = 0x440;
        inline uintptr_t ServerIP = 0x620;
        inline uintptr_t ToRenderView1 = 0x1d8;
        inline uintptr_t ToRenderView2 = 0x8;
        inline uintptr_t ToRenderView3 = 0x28;
        inline uintptr_t workspace = 0x178;
    }

    namespace depth {
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t FarIntensity = 0xd0;
        inline uintptr_t FocusDistance = 0xd4;
        inline uintptr_t InFocusRadius = 0xd8;
        inline uintptr_t NearIntensity = 0xdc;
    }

    namespace drag {
        inline uintptr_t ActivatedCursorIcon = 0x1d8;
        inline uintptr_t CursorIcon = 0xe0;
        inline uintptr_t MaxActivationDistance = 0x100;
        inline uintptr_t MaxDragAngle = 0x2c0;
        inline uintptr_t MaxDragTranslation = 0x284;
        inline uintptr_t MaxForce = 0x2c4;
        inline uintptr_t MaxTorque = 0x2c8;
        inline uintptr_t MinDragAngle = 0x2cc;
        inline uintptr_t MinDragTranslation = 0x290;
        inline uintptr_t ReferenceInstance = 0x208;
        inline uintptr_t Responsiveness = 0x2d8;
    }

    namespace fakemodel {
        inline uintptr_t Pointer = 0x78ff228;
        inline uintptr_t RealDataModel = 0x1d0;
    }

    namespace gui2d {
        inline uintptr_t AbsolutePosition = 0x110;
        inline uintptr_t AbsoluteRotation = 0x188;
        inline uintptr_t AbsoluteSize = 0x118;
    }

    namespace gui {
        inline uintptr_t BackgroundColor3 = 0x540;
        inline uintptr_t BackgroundTransparency = 0x54c;
        inline uintptr_t BorderColor3 = 0x54c;
        inline uintptr_t Image = 0x988;
        inline uintptr_t LayoutOrder = 0x580;
        inline uintptr_t Position = 0x510;
        inline uintptr_t RichText = 0xb58;
        inline uintptr_t Rotation = 0x188;
        inline uintptr_t ScreenGui_Enabled = 0x4c4;
        inline uintptr_t Size = 0x530;
        inline uintptr_t text = 0xda0;
        inline uintptr_t TextColor3 = 0xe50;
        inline uintptr_t Visible = 0x5ad;
        inline uintptr_t ZIndex = 0x19b;
    }

    namespace humanoid {
        inline uintptr_t AutoJumpEnabled = 0x1e0;
        inline uintptr_t AutoRotate = 0x1e1;
        inline uintptr_t AutomaticScalingEnabled = 0xde;
        inline uintptr_t BreakJointsOnDeath = 0x1e3;
        inline uintptr_t CameraOffset = 0x140;
        inline uintptr_t DisplayDistanceType = 0x18c;
        inline uintptr_t DisplayName = 0xd0;
        inline uintptr_t EvaluateStateMachine = 0x1e4;
        inline uintptr_t FloorMaterial = 0x190;
        inline uintptr_t Health = 0x194;
        inline uintptr_t HealthDisplayDistance = 0x198;
        inline uintptr_t HealthDisplayType = 0x19c;
        inline uintptr_t HipHeight = 0x1a0;
        inline uintptr_t HumanoidRootPart = 0x480;
        inline uintptr_t HumanoidState = 0x8a0;
        inline uintptr_t HumanoidStateID = 0x20;
        inline uintptr_t IsWalking = 0x91f;
        inline uintptr_t Jump = 0x1e6;
        inline uintptr_t JumpHeight = 0x1ac;
        inline uintptr_t JumpPower = 0x1b0;
        inline uintptr_t MaxHealth = 0x1b4;
        inline uintptr_t MaxSlopeAngle = 0x1b8;
        inline uintptr_t MoveDirection = 0x158;
        inline uintptr_t MoveToPart = 0x130;
        inline uintptr_t MoveToPoint = 0x17c;
        inline uintptr_t NameDisplayDistance = 0x1bc;
        inline uintptr_t NameOcclusion = 0x1c0;
        inline uintptr_t PlatformStand = 0x1e8;
        inline uintptr_t PlatformStatePointer = 0x262CF636;
        inline uintptr_t RequiresNeck = 0x1e9;
        inline uintptr_t RigType = 0x1cc;
        inline uintptr_t SeatPart = 0x120;
        inline uintptr_t Sit = 0x1e9;
        inline uintptr_t TargetPoint = 0x164;
        inline uintptr_t UseJumpPower = 0x1ec;
        inline uintptr_t WalkTimer = 0x410;
        inline uintptr_t walkspeed = 0x1dc;
        inline uintptr_t WalkspeedCheck = 0x3c4;
    }

    namespace instance {
        inline uintptr_t AttributeContainer = 0x48;
        inline uintptr_t AttributeList = 0x18;
        inline uintptr_t AttributeToNext = 0x58;
        inline uintptr_t AttributeToValue = 0x18;
        inline uintptr_t ChildrenEnd = 0x8;
        inline uintptr_t ChildrenStart = 0x78;
        inline uintptr_t ClassBase = 0x4920;
        inline uintptr_t ClassDescriptor = 0x18;
        inline uintptr_t ClassName = 0x8;
        inline uintptr_t name = 0xb0;
        inline uintptr_t parent = 0x70;
        inline uintptr_t This = 0x8;
    }

    namespace light {
        inline uintptr_t Ambient = 0xe0;
        inline uintptr_t Brightness = 0x128;
        inline uintptr_t ClockTime = 0x1c0;
        inline uintptr_t ColorShift_Bottom = 0xf8;
        inline uintptr_t ColorShift_Top = 0xec;
        inline uintptr_t EnvironmentDiffuseScale = 0x12c;
        inline uintptr_t EnvironmentSpecularScale = 0x130;
        inline uintptr_t ExposureCompensation = 0x134;
        inline uintptr_t FogColor = 0x104;
        inline uintptr_t FogEnd = 0x13c;
        inline uintptr_t FogStart = 0x140;
        inline uintptr_t GeographicLatitude = 0x198;
        inline uintptr_t GlobalShadows = 0x150;
        inline uintptr_t GradientBottom = 0x19c;
        inline uintptr_t GradientTop = 0x158;
        inline uintptr_t LightColor = 0x164;
        inline uintptr_t LightDirection = 0x170;
        inline uintptr_t MoonPosition = 0x18c;
        inline uintptr_t OutdoorAmbient = 0x110;
        inline uintptr_t sky = 0x1e0;
        inline uintptr_t Source = 0x17c;
        inline uintptr_t SunPosition = 0x180;
    }

    namespace localscript {
        inline uintptr_t bytecode = 0x1a8;
        inline uintptr_t GUID = 0xe8;
        inline uintptr_t Hash = 0x1b8;
    }

    namespace material {
        inline uintptr_t Asphalt = 0x30;
        inline uintptr_t Basalt = 0x27;
        inline uintptr_t Brick = 0xf;
        inline uintptr_t Cobblestone = 0x33;
        inline uintptr_t Concrete = 0xc;
        inline uintptr_t CrackedLava = 0x2d;
        inline uintptr_t Glacier = 0x1b;
        inline uintptr_t Grass = 0x6;
        inline uintptr_t Ground = 0x2a;
        inline uintptr_t Ice = 0x36;
        inline uintptr_t LeafyGrass = 0x39;
        inline uintptr_t Limestone = 0x3f;
        inline uintptr_t Mud = 0x24;
        inline uintptr_t Pavement = 0x42;
        inline uintptr_t Rock = 0x18;
        inline uintptr_t Salt = 0x3c;
        inline uintptr_t Sand = 0x12;
        inline uintptr_t Sandstone = 0x21;
        inline uintptr_t Slate = 0x9;
        inline uintptr_t Snow = 0x1e;
        inline uintptr_t WoodPlanks = 0x15;
    }

    namespace meshprovider {
        inline uintptr_t AssetID = 0x10;
        inline uintptr_t cache = 0xe8;
        inline uintptr_t LRUCache = 0x20;
        inline uintptr_t meshdata = 0x40;
        inline uintptr_t ToMeshData = 0x40;
    }

    namespace meshdata {
        inline uintptr_t FaceEnd = 0x38;
        inline uintptr_t FaceStart = 0x30;
        inline uintptr_t VertexEnd = 0x8;
        inline uintptr_t VertexStart = 0x0;
    }

    namespace meshpart {
        inline uintptr_t MeshId = 0x300;
        inline uintptr_t Texture = 0x330;
    }

    namespace misc {
        inline uintptr_t Adornee = 0x108;
        inline uintptr_t AnimationId = 0xd0;
        inline uintptr_t StringLength = 0x10;
        inline uintptr_t Value = 0xd0;
    }

    namespace model {
        inline uintptr_t PrimaryPart = 0x278;
        inline uintptr_t Scale = 0x164;
    }

    namespace modulescript {
        inline uintptr_t bytecode = 0x150;
        inline uintptr_t GUID = 0xe8;
        inline uintptr_t Hash = 0x160;
        inline uintptr_t IsCoreScript = 0x0;
    }

    namespace mouseservice {
        inline uintptr_t InputObject = 0x108;
        inline uintptr_t InputObject2 = 0x118;
        inline uintptr_t MousePosition = 0xec;
        inline uintptr_t SensitivityPointer = 0x307;
    }

    namespace particle {
        inline uintptr_t Acceleration = 0x1f0;
        inline uintptr_t Brightness = 0x22c;
        inline uintptr_t Drag = 0x230;
        inline uintptr_t Lifetime = 0x204;
        inline uintptr_t LightEmission = 0x248;
        inline uintptr_t LightInfluence = 0x24c;
        inline uintptr_t Rate = 0x258;
        inline uintptr_t RotSpeed = 0x20c;
        inline uintptr_t Rotation = 0x214;
        inline uintptr_t Speed = 0x21c;
        inline uintptr_t SpreadAngle = 0x224;
        inline uintptr_t Texture = 0x1d0;
        inline uintptr_t TimeScale = 0x26c;
        inline uintptr_t VelocityInheritance = 0x270;
        inline uintptr_t ZOffset = 0x274;
    }

    namespace player {
        inline uintptr_t AccountAge = 0x32c;
        inline uintptr_t CameraMode = 0x338;
        inline uintptr_t DisplayName = 0x130;
        inline uintptr_t HealthDisplayDistance = 0x358;
        inline uintptr_t LocalPlayer = 0x138;
        inline uintptr_t LocaleId = 0x110;
        inline uintptr_t MaxZoomDistance = 0x330;
        inline uintptr_t MinZoomDistance = 0x334;
        inline uintptr_t ModelInstance = 0x3a8;
        inline uintptr_t mouse = 0x1180;
        inline uintptr_t NameDisplayDistance = 0x368;
        inline uintptr_t team = 0x2b0;
        inline uintptr_t TeamColor = 0x374;
        inline uintptr_t UserId = 0x2d8;
    }

    namespace playerconfig {
        inline uintptr_t Pointer = 0x0;
    }

    namespace playermouse {
        inline uintptr_t icon = 0xe0;
        inline uintptr_t workspace = 0x168;
    }

    namespace primitive {
        inline uintptr_t AssemblyAngularVelocity = 0x104;
        inline uintptr_t AssemblyLinearVelocity = 0xf8;
        inline uintptr_t Flags = 0x1b6;
        inline uintptr_t Material = 0x0;
        inline uintptr_t Owner = 0x200;
        inline uintptr_t Position = 0xec;
        inline uintptr_t Rotation = 0xc8;
        inline uintptr_t Size = 0x1b8;
        inline uintptr_t Validate = 0x6;
    }

    namespace primitiveflag {
        inline uintptr_t Anchored = 0x2;
        inline uintptr_t CanCollide = 0x8;
        inline uintptr_t CanQuery = 0x20;
        inline uintptr_t CanTouch = 0x10;
    }

    namespace prompt {
        inline uintptr_t ActionText = 0xc8;
        inline uintptr_t Enabled = 0x14e;
        inline uintptr_t GamepadKeyCode = 0x134;
        inline uintptr_t HoldDuration = 0x138;
        inline uintptr_t KeyCode = 0x13c;
        inline uintptr_t MaxActivationDistance = 0x140;
        inline uintptr_t ObjectText = 0xe8;
        inline uintptr_t RequiresLineOfSight = 0x14f;
    }

    namespace job {
        inline uintptr_t fakemodel = 0x38;
        inline uintptr_t RealDataModel = 0x1c0;
        inline uintptr_t view = 0x1d0;
    }

    namespace view {
        inline uintptr_t DeviceD3D11 = 0x8;
        inline uintptr_t LightingValid = 0x150;
        inline uintptr_t SkyValid = 0x28d;
        inline uintptr_t render = 0x10;
    }

    namespace run {
        inline uintptr_t HeartbeatFPS = 0xb8;
        inline uintptr_t HeartbeatTask = 0xf8;
    }

    namespace script {
        inline uintptr_t bytecode = 0x1a8;
        inline uintptr_t GUID = 0xe8;
        inline uintptr_t Hash = 0x1b8;
    }

    namespace scriptcontext {
        inline uintptr_t RequireBypass = 0x0;
    }

    namespace seat {
        inline uintptr_t Occupant = 0x220;
    }

    namespace sky {
        inline uintptr_t MoonAngularSize = 0x25c;
        inline uintptr_t MoonTextureId = 0xe0;
        inline uintptr_t SkyboxBk = 0x110;
        inline uintptr_t SkyboxDn = 0x140;
        inline uintptr_t SkyboxFt = 0x170;
        inline uintptr_t SkyboxLf = 0x1a0;
        inline uintptr_t SkyboxOrientation = 0x250;
        inline uintptr_t SkyboxRt = 0x1d0;
        inline uintptr_t SkyboxUp = 0x200;
        inline uintptr_t StarCount = 0x260;
        inline uintptr_t SunAngularSize = 0x254;
        inline uintptr_t SunTextureId = 0x230;
    }

    namespace sound {
        inline uintptr_t Looped = 0x155;
        inline uintptr_t PlaybackSpeed = 0x134;
        inline uintptr_t Playing = 0x158;
        inline uintptr_t RollOffMaxDistance = 0x138;
        inline uintptr_t RollOffMinDistance = 0x13c;
        inline uintptr_t SoundGroup = 0x100;
        inline uintptr_t SoundId = 0xe0;
        inline uintptr_t Volume = 0x148;
    }

    namespace spawn {
        inline uintptr_t AllowTeamChangeOnTouch = 0x1f8;
        inline uintptr_t Enabled = 0x1f9;
        inline uintptr_t ForcefieldDuration = 0x1f0;
        inline uintptr_t Neutral = 0x1fa;
        inline uintptr_t TeamColor = 0x1f4;
    }

    namespace specialmesh {
        inline uintptr_t MeshId = 0x108;
        inline uintptr_t Scale = 0xdc;
    }

    namespace stat {
        inline uintptr_t Value = 0xc8;
    }

    namespace sunray {
        inline uintptr_t Enabled = 0xc8;
        inline uintptr_t Intensity = 0xd0;
        inline uintptr_t Spread = 0xd4;
    }

    namespace surface {
        inline uintptr_t AlphaMode = 0x2a0;
        inline uintptr_t tocolor = 0x288;
        inline uintptr_t ColorMap = 0xe0;
        inline uintptr_t EmissiveMaskContent = 0x110;
        inline uintptr_t EmissiveStrength = 0x2a4;
        inline uintptr_t EmissiveTint = 0x294;
        inline uintptr_t MetalnessMap = 0x140;
        inline uintptr_t NormalMap = 0x170;
        inline uintptr_t RoughnessMap = 0x1a0;
    }

    namespace task {
        inline uintptr_t JobEnd = 0xd0;
        inline uintptr_t JobName = 0x18;
        inline uintptr_t JobStart = 0xc8;
        inline uintptr_t MaxFPS = 0xb0;
        inline uintptr_t Pointer = 0x7e90548;
    }

    namespace team {
        inline uintptr_t BrickColor = 0xd0;
    }

    namespace terrain {
        inline uintptr_t GrassLength = 0x1f8;
        inline uintptr_t material = 0x2a8;
        inline uintptr_t WaterColor = 0x1e8;
        inline uintptr_t WaterReflectance = 0x200;
        inline uintptr_t WaterTransparency = 0x204;
        inline uintptr_t WaterWaveSize = 0x208;
        inline uintptr_t WaterWaveSpeed = 0x20c;
    }

    namespace texture {
        inline uintptr_t Decal_Texture = 0x198;
        inline uintptr_t Texture_Texture = 0x198;
    }

    namespace tool {
        inline uintptr_t CanBeDropped = 0x4c8;
        inline uintptr_t Enabled = 0x4c9;
        inline uintptr_t Grip = 0x4bc;
        inline uintptr_t ManualActivationOnly = 0x4ca;
        inline uintptr_t RequiresHandle = 0x4cb;
        inline uintptr_t TextureId = 0x370;
        inline uintptr_t Tooltip = 0x478;
    }

    namespace unionop {
        inline uintptr_t AssetId = 0x2f8;
    }

    namespace input {
        inline uintptr_t inputstate = 0x2d8;
    }

    namespace vehicle {
        inline uintptr_t MaxSpeed = 0x238;
        inline uintptr_t SteerFloat = 0x240;
        inline uintptr_t ThrottleFloat = 0x248;
        inline uintptr_t Torque = 0x24c;
        inline uintptr_t TurnSpeed = 0x250;
    }

    namespace inputstate {
        inline uintptr_t CapsLock = 0x40;
        inline uintptr_t CurrentTextBox = 0x48;
    }

    namespace workspace {
        inline uintptr_t CurrentCamera = 0x4b0;
        inline uintptr_t DistributedGameTime = 0x4d0;
        inline uintptr_t ReadOnlyGravity = 0x9f0;
        inline uintptr_t world = 0x408;
    }

    namespace world {
        inline uintptr_t air = 0x218;
        inline uintptr_t FallenPartsDestroyHeight = 0x208;
        inline uintptr_t Gravity = 0x210;
        inline uintptr_t GravityOverride = 0x658;
        inline uintptr_t Primitives = 0x280;
        inline uintptr_t worldStepsPerSec = 0x678;
    }

    namespace render {
        inline uintptr_t Dimensions = 0xab0;
        inline uintptr_t fakemodel = 0xa90;
        inline uintptr_t Pointer = 0x801dfb0;
        inline uintptr_t view = 0xbb0;
        inline uintptr_t ViewMatrix = 0x150;
    }

    namespace silent {
         inline uintptr_t FramePositionOffsetX = 0x524;
         inline uintptr_t FramePositionOffsetY = 0x52C;
         inline uintptr_t FramePositionX = 0x520;
         inline uintptr_t FramePositionY = 0x528;
         inline uintptr_t FrameRotation = 0x188;
         inline uintptr_t FrameSizeOffsetX = 0x540;
         inline uintptr_t FrameSizeOffsetY = 0x544;
         inline uintptr_t FrameSizeX = 0x538;
         inline uintptr_t FrameSizeY = 0x53C;
    }

    namespace flag {
         inline uintptr_t List = 0x7ce33d8;
         inline uintptr_t ToValueGetSet = 0x30;
    }

    namespace value {
         inline uintptr_t ToValue = 0xc0;
    }

// init() - populate runtime offsets from OffsetsManager (fallback to defaults)
inline void init() {
    auto& mgr = OffsetsManager::instance();
    if (!mgr.is_loaded()) return;

#define TRY(ns, field) do { offset::ns::field = mgr.get_offset(#ns, #field); } while(0)

    // ===== DEBUG: verify critical offsets loaded correctly =====
    printf("[OFFSETS] instance::ChildrenStart = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::ChildrenStart,
        (unsigned long long)mgr.get_offset("instance", "ChildrenStart"));
    printf("[OFFSETS] instance::ChildrenEnd   = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::ChildrenEnd,
        (unsigned long long)mgr.get_offset("instance", "ChildrenEnd"));
    printf("[OFFSETS] instance::ClassDescriptor = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::ClassDescriptor,
        (unsigned long long)mgr.get_offset("instance", "ClassDescriptor"));
    printf("[OFFSETS] instance::ClassName = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::ClassName,
        (unsigned long long)mgr.get_offset("instance", "ClassName"));
    printf("[OFFSETS] instance::name = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::name,
        (unsigned long long)mgr.get_offset("instance", "name"));
    printf("[OFFSETS] instance::parent = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::instance::parent,
        (unsigned long long)mgr.get_offset("instance", "parent"));
    printf("[OFFSETS] fakemodel::Pointer = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::fakemodel::Pointer,
        (unsigned long long)mgr.get_offset("fakemodel", "Pointer"));
    printf("[OFFSETS] fakemodel::RealDataModel = 0x%llx (JSON: 0x%llx)\n",
        (unsigned long long)offset::fakemodel::RealDataModel,
        (unsigned long long)mgr.get_offset("fakemodel", "RealDataModel"));
    // ===== END DEBUG =====

    TRY(air, AirDensity);
    TRY(air, GlobalWind);
    TRY(animation, Animation);
    TRY(animation, animator);
    TRY(animation, IsPlaying);
    TRY(animation, Looped);
    TRY(animation, Speed);
    TRY(animation, TimePosition);
    TRY(animator, ActiveAnimations);
    TRY(atmosphere, Decay);
    TRY(atmosphere, Density);
    TRY(atmosphere, Glare);
    TRY(atmosphere, Haze);
    TRY(atmosphere, Offset);
    TRY(atmosphere, tocolor);
    TRY(attachment, Position);
    TRY(basepart, CastShadow);
    TRY(basepart, Color3);
    TRY(basepart, Locked);
    TRY(basepart, Massless);
    TRY(basepart, primitive);
    TRY(basepart, Reflectance);
    TRY(basepart, Shape);
    TRY(basepart, Transparency);
    TRY(beam, Attachment0);
    TRY(beam, Attachment1);
    TRY(beam, Brightness);
    TRY(beam, CurveSize0);
    TRY(beam, CurveSize1);
    TRY(beam, LightEmission);
    TRY(beam, LightInfluence);
    TRY(beam, Texture);
    TRY(beam, TextureLength);
    TRY(beam, TextureSpeed);
    TRY(beam, Width0);
    TRY(beam, Width1);
    TRY(beam, ZOffset);
    TRY(bloom, Enabled);
    TRY(bloom, Intensity);
    TRY(bloom, Size);
    TRY(bloom, Threshold);
    TRY(blur, Enabled);
    TRY(blur, Size);
    TRY(bytecode, Pointer);
    TRY(bytecode, Size);
    TRY(camera, CameraSubject);
    TRY(camera, CameraType);
    TRY(camera, FieldOfView);
    TRY(camera, ImagePlaneDepth);
    TRY(camera, Position);
    TRY(camera, Rotation);
    TRY(camera, Viewport);
    TRY(camera, ViewportSize);
    TRY(charmesh, BaseTextureId);
    TRY(charmesh, BodyPart);
    TRY(charmesh, MeshId);
    TRY(charmesh, OverlayTextureId);
    TRY(click, MaxActivationDistance);
    TRY(click, MouseIcon);
    TRY(clothing, Color3);
    TRY(clothing, Template);
    TRY(correction, Brightness);
    TRY(correction, Contrast);
    TRY(correction, Enabled);
    TRY(correction, TintColor);
    TRY(datamodel, CreatorId);
    TRY(datamodel, GameId);
    TRY(datamodel, GameLoaded);
    TRY(datamodel, JobId);
    TRY(datamodel, PlaceId);
    TRY(datamodel, PlaceVersion);
    TRY(datamodel, PrimitiveCount);
    TRY(datamodel, scriptcontext);
    TRY(datamodel, ServerIP);
    TRY(datamodel, ToRenderView1);
    TRY(datamodel, ToRenderView2);
    TRY(datamodel, ToRenderView3);
    TRY(datamodel, workspace);
    TRY(depth, Enabled);
    TRY(depth, FarIntensity);
    TRY(depth, FocusDistance);
    TRY(depth, InFocusRadius);
    TRY(depth, NearIntensity);
    TRY(drag, ActivatedCursorIcon);
    TRY(drag, CursorIcon);
    TRY(drag, MaxActivationDistance);
    TRY(drag, MaxDragAngle);
    TRY(drag, MaxDragTranslation);
    TRY(drag, MaxForce);
    TRY(drag, MaxTorque);
    TRY(drag, MinDragAngle);
    TRY(drag, MinDragTranslation);
    TRY(drag, ReferenceInstance);
    TRY(drag, Responsiveness);
    TRY(fakemodel, Pointer);
    TRY(fakemodel, RealDataModel);
    TRY(flag, List);
    TRY(flag, ToValueGetSet);
    TRY(grading, Enabled);
    TRY(grading, TonemapperPreset);
    TRY(gui, BackgroundColor3);
    TRY(gui, BackgroundTransparency);
    TRY(gui, BorderColor3);
    TRY(gui, Image);
    TRY(gui, LayoutOrder);
    TRY(gui, Position);
    TRY(gui, RichText);
    TRY(gui, Rotation);
    TRY(gui, ScreenGui_Enabled);
    TRY(gui, Size);
    TRY(gui, text);
    TRY(gui, TextColor3);
    TRY(gui, Visible);
    TRY(gui, ZIndex);
    TRY(gui2d, AbsolutePosition);
    TRY(gui2d, AbsoluteRotation);
    TRY(gui2d, AbsoluteSize);
    TRY(humanoid, AutoJumpEnabled);
    TRY(humanoid, AutoRotate);
    TRY(humanoid, AutomaticScalingEnabled);
    TRY(humanoid, BreakJointsOnDeath);
    TRY(humanoid, CameraOffset);
    TRY(humanoid, DisplayDistanceType);
    TRY(humanoid, DisplayName);
    TRY(humanoid, EvaluateStateMachine);
    TRY(humanoid, FloorMaterial);
    TRY(humanoid, Health);
    TRY(humanoid, HealthDisplayDistance);
    TRY(humanoid, HealthDisplayType);
    TRY(humanoid, HipHeight);
    TRY(humanoid, HumanoidRootPart);
    TRY(humanoid, HumanoidState);
    TRY(humanoid, HumanoidStateID);
    TRY(humanoid, IsWalking);
    TRY(humanoid, Jump);
    TRY(humanoid, JumpHeight);
    TRY(humanoid, JumpPower);
    TRY(humanoid, MaxHealth);
    TRY(humanoid, MaxSlopeAngle);
    TRY(humanoid, MoveDirection);
    TRY(humanoid, MoveToPart);
    TRY(humanoid, MoveToPoint);
    TRY(humanoid, NameDisplayDistance);
    TRY(humanoid, NameOcclusion);
    TRY(humanoid, PlatformStand);
    TRY(humanoid, PlatformStatePointer);
    TRY(humanoid, RequiresNeck);
    TRY(humanoid, RigType);
    TRY(humanoid, SeatPart);
    TRY(humanoid, Sit);
    TRY(humanoid, TargetPoint);
    TRY(humanoid, UseJumpPower);
    TRY(humanoid, WalkTimer);
    TRY(humanoid, walkspeed);
    TRY(humanoid, WalkspeedCheck);
    TRY(input, inputstate);
    TRY(inputstate, CapsLock);
    TRY(inputstate, CurrentTextBox);
    TRY(instance, AttributeContainer);
    TRY(instance, AttributeList);
    TRY(instance, AttributeToNext);
    TRY(instance, AttributeToValue);
    TRY(instance, ChildrenEnd);
    TRY(instance, ChildrenStart);
    TRY(instance, ClassBase);
    TRY(instance, ClassDescriptor);
    TRY(instance, ClassName);
    TRY(instance, name);
    TRY(instance, parent);
    TRY(instance, This);
    TRY(job, fakemodel);
    TRY(job, RealDataModel);
    TRY(job, view);
    TRY(light, Ambient);
    TRY(light, Brightness);
    TRY(light, ClockTime);
    TRY(light, ColorShift_Bottom);
    TRY(light, ColorShift_Top);
    TRY(light, EnvironmentDiffuseScale);
    TRY(light, EnvironmentSpecularScale);
    TRY(light, ExposureCompensation);
    TRY(light, FogColor);
    TRY(light, FogEnd);
    TRY(light, FogStart);
    TRY(light, GeographicLatitude);
    TRY(light, GlobalShadows);
    TRY(light, GradientBottom);
    TRY(light, GradientTop);
    TRY(light, LightColor);
    TRY(light, LightDirection);
    TRY(light, MoonPosition);
    TRY(light, OutdoorAmbient);
    TRY(light, sky);
    TRY(light, Source);
    TRY(light, SunPosition);
    TRY(localscript, GUID);
    TRY(localscript, Hash);
    TRY(localscript, bytecode);
    TRY(material, Asphalt);
    TRY(material, Basalt);
    TRY(material, Brick);
    TRY(material, Cobblestone);
    TRY(material, Concrete);
    TRY(material, CrackedLava);
    TRY(material, Glacier);
    TRY(material, Grass);
    TRY(material, Ground);
    TRY(material, Ice);
    TRY(material, LeafyGrass);
    TRY(material, Limestone);
    TRY(material, Mud);
    TRY(material, Pavement);
    TRY(material, Rock);
    TRY(material, Salt);
    TRY(material, Sand);
    TRY(material, Sandstone);
    TRY(material, Slate);
    TRY(material, Snow);
    TRY(material, WoodPlanks);
    TRY(meshdata, FaceEnd);
    TRY(meshdata, FaceStart);
    TRY(meshdata, VertexEnd);
    TRY(meshdata, VertexStart);
    TRY(meshpart, MeshId);
    TRY(meshpart, Texture);
    TRY(meshprovider, AssetID);
    TRY(meshprovider, cache);
    TRY(meshprovider, LRUCache);
    TRY(meshprovider, meshdata);
    TRY(meshprovider, ToMeshData);
    TRY(misc, Adornee);
    TRY(misc, AnimationId);
    TRY(misc, StringLength);
    TRY(misc, Value);
    TRY(model, PrimaryPart);
    TRY(model, Scale);
    TRY(modulescript, GUID);
    TRY(modulescript, Hash);
    TRY(modulescript, IsCoreScript);
    TRY(modulescript, bytecode);
    TRY(mouseservice, InputObject);
    TRY(mouseservice, InputObject2);
    TRY(mouseservice, MousePosition);
    TRY(mouseservice, SensitivityPointer);
    TRY(particle, Acceleration);
    TRY(particle, Brightness);
    TRY(particle, Drag);
    TRY(particle, Lifetime);
    TRY(particle, LightEmission);
    TRY(particle, LightInfluence);
    TRY(particle, Rate);
    TRY(particle, RotSpeed);
    TRY(particle, Rotation);
    TRY(particle, Speed);
    TRY(particle, SpreadAngle);
    TRY(particle, Texture);
    TRY(particle, TimeScale);
    TRY(particle, VelocityInheritance);
    TRY(particle, ZOffset);
    TRY(player, AccountAge);
    TRY(player, CameraMode);
    TRY(player, DisplayName);
    TRY(player, HealthDisplayDistance);
    TRY(player, LocalPlayer);
    TRY(player, LocaleId);
    TRY(player, MaxZoomDistance);
    TRY(player, MinZoomDistance);
    TRY(player, ModelInstance);
    TRY(player, mouse);
    TRY(player, NameDisplayDistance);
    TRY(player, team);
    TRY(player, TeamColor);
    TRY(player, UserId);
    TRY(playerconfig, Pointer);
    TRY(playermouse, icon);
    TRY(playermouse, workspace);
    TRY(primitive, AssemblyAngularVelocity);
    TRY(primitive, AssemblyLinearVelocity);
    TRY(primitive, Flags);
    TRY(primitive, Material);
    TRY(primitive, Owner);
    TRY(primitive, Position);
    TRY(primitive, Rotation);
    TRY(primitive, Size);
    TRY(primitive, Validate);
    TRY(primitiveflag, Anchored);
    TRY(primitiveflag, CanCollide);
    TRY(primitiveflag, CanQuery);
    TRY(primitiveflag, CanTouch);
    TRY(prompt, ActionText);
    TRY(prompt, Enabled);
    TRY(prompt, GamepadKeyCode);
    TRY(prompt, HoldDuration);
    TRY(prompt, KeyCode);
    TRY(prompt, MaxActivationDistance);
    TRY(prompt, ObjectText);
    TRY(prompt, RequiresLineOfSight);
    TRY(render, Dimensions);
    TRY(render, fakemodel);
    TRY(render, Pointer);
    TRY(render, view);
    TRY(render, ViewMatrix);
    TRY(run, HeartbeatFPS);
    TRY(run, HeartbeatTask);
    TRY(script, GUID);
    TRY(script, Hash);
    TRY(script, bytecode);
    TRY(scriptcontext, RequireBypass);
    TRY(seat, Occupant);
    TRY(silent, FramePositionOffsetX);
    TRY(silent, FramePositionOffsetY);
    TRY(silent, FramePositionX);
    TRY(silent, FramePositionY);
    TRY(silent, FrameRotation);
    TRY(silent, FrameSizeOffsetX);
    TRY(silent, FrameSizeOffsetY);
    TRY(silent, FrameSizeX);
    TRY(silent, FrameSizeY);
    TRY(sky, MoonAngularSize);
    TRY(sky, MoonTextureId);
    TRY(sky, SkyboxBk);
    TRY(sky, SkyboxDn);
    TRY(sky, SkyboxFt);
    TRY(sky, SkyboxLf);
    TRY(sky, SkyboxOrientation);
    TRY(sky, SkyboxRt);
    TRY(sky, SkyboxUp);
    TRY(sky, StarCount);
    TRY(sky, SunAngularSize);
    TRY(sky, SunTextureId);
    TRY(sound, Looped);
    TRY(sound, PlaybackSpeed);
    TRY(sound, Playing);
    TRY(sound, RollOffMaxDistance);
    TRY(sound, RollOffMinDistance);
    TRY(sound, SoundGroup);
    TRY(sound, SoundId);
    TRY(sound, Volume);
    TRY(spawn, AllowTeamChangeOnTouch);
    TRY(spawn, Enabled);
    TRY(spawn, ForcefieldDuration);
    TRY(spawn, Neutral);
    TRY(spawn, TeamColor);
    TRY(specialmesh, MeshId);
    TRY(specialmesh, Scale);
    TRY(stat, Value);
    TRY(sunray, Enabled);
    TRY(sunray, Intensity);
    TRY(sunray, Spread);
    TRY(surface, AlphaMode);
    TRY(surface, ColorMap);
    TRY(surface, EmissiveMaskContent);
    TRY(surface, EmissiveStrength);
    TRY(surface, EmissiveTint);
    TRY(surface, MetalnessMap);
    TRY(surface, NormalMap);
    TRY(surface, RoughnessMap);
    TRY(surface, tocolor);
    TRY(task, JobEnd);
    TRY(task, JobName);
    TRY(task, JobStart);
    TRY(task, MaxFPS);
    TRY(task, Pointer);
    TRY(team, BrickColor);
    TRY(terrain, GrassLength);
    TRY(terrain, material);
    TRY(terrain, WaterColor);
    TRY(terrain, WaterReflectance);
    TRY(terrain, WaterTransparency);
    TRY(terrain, WaterWaveSize);
    TRY(terrain, WaterWaveSpeed);
    TRY(texture, Decal_Texture);
    TRY(texture, Texture_Texture);
    TRY(tool, CanBeDropped);
    TRY(tool, Enabled);
    TRY(tool, Grip);
    TRY(tool, ManualActivationOnly);
    TRY(tool, RequiresHandle);
    TRY(tool, TextureId);
    TRY(tool, Tooltip);
    TRY(unionop, AssetId);
    TRY(value, ToValue);
    TRY(vehicle, MaxSpeed);
    TRY(vehicle, SteerFloat);
    TRY(vehicle, ThrottleFloat);
    TRY(vehicle, Torque);
    TRY(vehicle, TurnSpeed);
    TRY(view, DeviceD3D11);
    TRY(view, LightingValid);
    TRY(view, render);
    TRY(view, SkyValid);
    TRY(workspace, CurrentCamera);
    TRY(workspace, DistributedGameTime);
    TRY(workspace, ReadOnlyGravity);
    TRY(workspace, world);
    TRY(world, air);
    TRY(world, FallenPartsDestroyHeight);
    TRY(world, Gravity);
    TRY(world, GravityOverride);
    TRY(world, Primitives);
    TRY(world, worldStepsPerSec);

#undef TRY
}

}
