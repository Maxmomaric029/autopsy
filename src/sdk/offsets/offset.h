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
// NOTE: Endpoint uses Roblox internal class names (PascalCase) which may differ
// from C++ namespace names. Use MAP() when names differ, TRY() when they match.
inline void init() {
    auto& mgr = OffsetsManager::instance();
    if (!mgr.is_loaded()) return;

    // TRY: endpoint class/field name matches C++ (case-insensitive)
#define TRY(ns, field) do { auto v = mgr.get_offset(#ns, #field); if (v) offset::ns::field = v; } while(0)
    // MAP: endpoint class/field name differs from C++
#define MAP(ec, ef, ns, fld) do { auto v = mgr.get_offset(#ec, #ef); if (v) offset::ns::fld = v; } while(0)

    // ===== FakeDataModel (C++: fakemodel) =====
    MAP(FakeDataModel, Pointer, fakemodel, Pointer);
    MAP(FakeDataModel, RealDataModel, fakemodel, RealDataModel);

    // ===== VisualEngine (C++: render) =====
    MAP(VisualEngine, Pointer, render, Pointer);
    MAP(VisualEngine, Dimensions, render, Dimensions);
    MAP(VisualEngine, ViewMatrix, render, ViewMatrix);
    MAP(VisualEngine, FakeDataModel, render, fakemodel);
    MAP(VisualEngine, RenderView, render, view);

    // ===== RenderView (C++: view) =====
    MAP(RenderView, DeviceD3D11, view, DeviceD3D11);
    MAP(RenderView, LightingValid, view, LightingValid);
    MAP(RenderView, SkyValid, view, SkyValid);
    MAP(RenderView, VisualEngine, view, render);

    // ===== RenderJob (C++: job) =====
    MAP(RenderJob, FakeDataModel, job, fakemodel);
    MAP(RenderJob, RealDataModel, job, RealDataModel);
    MAP(RenderJob, RenderView, job, view);

    // ===== TaskScheduler (C++: task) =====
    MAP(TaskScheduler, Pointer, task, Pointer);
    MAP(TaskScheduler, JobStart, task, JobStart);
    MAP(TaskScheduler, JobEnd, task, JobEnd);
    MAP(TaskScheduler, JobName, task, JobName);
    MAP(TaskScheduler, MaxFPS, task, MaxFPS);

    // ===== Lighting (C++: light) =====
    MAP(Lighting, Ambient, light, Ambient);
    MAP(Lighting, Brightness, light, Brightness);
    MAP(Lighting, ClockTime, light, ClockTime);
    MAP(Lighting, ColorShift_Bottom, light, ColorShift_Bottom);
    MAP(Lighting, ColorShift_Top, light, ColorShift_Top);
    MAP(Lighting, EnvironmentDiffuseScale, light, EnvironmentDiffuseScale);
    MAP(Lighting, EnvironmentSpecularScale, light, EnvironmentSpecularScale);
    MAP(Lighting, ExposureCompensation, light, ExposureCompensation);
    MAP(Lighting, FogColor, light, FogColor);
    MAP(Lighting, FogEnd, light, FogEnd);
    MAP(Lighting, FogStart, light, FogStart);
    MAP(Lighting, GeographicLatitude, light, GeographicLatitude);
    MAP(Lighting, GlobalShadows, light, GlobalShadows);
    MAP(Lighting, GradientBottom, light, GradientBottom);
    MAP(Lighting, GradientTop, light, GradientTop);
    MAP(Lighting, LightColor, light, LightColor);
    MAP(Lighting, LightDirection, light, LightDirection);
    MAP(Lighting, MoonPosition, light, MoonPosition);
    MAP(Lighting, OutdoorAmbient, light, OutdoorAmbient);
    MAP(Lighting, Sky, light, sky);
    MAP(Lighting, Source, light, Source);
    MAP(Lighting, SunPosition, light, SunPosition);

    // ===== GuiObject (C++: gui) =====
    MAP(GuiObject, ScreenGui_Enabled, gui, ScreenGui_Enabled);
    MAP(GuiObject, Position, gui, Position);
    MAP(GuiObject, Size, gui, Size);
    MAP(GuiObject, Visible, gui, Visible);
    MAP(GuiObject, Image, gui, Image);
    MAP(GuiObject, Text, gui, text);
    MAP(GuiObject, RichText, gui, RichText);
    MAP(GuiObject, BackgroundColor3, gui, BackgroundColor3);
    MAP(GuiObject, BorderColor3, gui, BorderColor3);
    MAP(GuiObject, TextColor3, gui, TextColor3);
    MAP(GuiObject, LayoutOrder, gui, LayoutOrder);
    MAP(GuiObject, ZIndex, gui, ZIndex);
    MAP(GuiObject, BackgroundTransparency, gui, BackgroundTransparency);
    MAP(GuiObject, Rotation, gui, Rotation);

    // ===== GuiBase2D (C++: gui2d) =====
    MAP(GuiBase2D, AbsolutePosition, gui2d, AbsolutePosition);
    MAP(GuiBase2D, AbsoluteRotation, gui2d, AbsoluteRotation);
    MAP(GuiBase2D, AbsoluteSize, gui2d, AbsoluteSize);

    // ===== AirProperties (C++: air) =====
    MAP(AirProperties, AirDensity, air, AirDensity);
    MAP(AirProperties, GlobalWind, air, GlobalWind);

    // ===== AnimationTrack (C++: animation) =====
    MAP(AnimationTrack, Animation, animation, Animation);
    MAP(AnimationTrack, Animator, animation, animator);
    MAP(AnimationTrack, IsPlaying, animation, IsPlaying);
    MAP(AnimationTrack, Looped, animation, Looped);
    MAP(AnimationTrack, Speed, animation, Speed);
    MAP(AnimationTrack, TimePosition, animation, TimePosition);

    // ===== UserInputService / WindowInputState (C++: input / inputstate) =====
    MAP(UserInputService, WindowInputState, input, inputstate);
    MAP(WindowInputState, CurrentTextBox, inputstate, CurrentTextBox);
    MAP(WindowInputState, CapsLock, inputstate, CapsLock);

    // ===== PlayerConfigurer (C++: playerconfig) =====
    MAP(PlayerConfigurer, Pointer, playerconfig, Pointer);

    // ===== PrimitiveFlags (C++: primitiveflag) =====
    MAP(PrimitiveFlags, Anchored, primitiveflag, Anchored);
    MAP(PrimitiveFlags, CanCollide, primitiveflag, CanCollide);
    MAP(PrimitiveFlags, CanTouch, primitiveflag, CanTouch);
    MAP(PrimitiveFlags, CanQuery, primitiveflag, CanQuery);

    // ===== BloomEffect (C++: bloom) =====
    MAP(BloomEffect, Intensity, bloom, Intensity);
    MAP(BloomEffect, Size, bloom, Size);
    MAP(BloomEffect, Threshold, bloom, Threshold);
    MAP(BloomEffect, Enabled, bloom, Enabled);

    // ===== BlurEffect (C++: blur) =====
    MAP(BlurEffect, Size, blur, Size);
    MAP(BlurEffect, Enabled, blur, Enabled);

    // ===== ColorCorrectionEffect (C++: correction) =====
    MAP(ColorCorrectionEffect, Brightness, correction, Brightness);
    MAP(ColorCorrectionEffect, Contrast, correction, Contrast);
    MAP(ColorCorrectionEffect, TintColor, correction, TintColor);
    MAP(ColorCorrectionEffect, Enabled, correction, Enabled);

    // ===== ColorGradingEffect (C++: grading) =====
    MAP(ColorGradingEffect, TonemapperPreset, grading, TonemapperPreset);
    MAP(ColorGradingEffect, Enabled, grading, Enabled);

    // ===== DepthOfFieldEffect (C++: depth) =====
    MAP(DepthOfFieldEffect, FarIntensity, depth, FarIntensity);
    MAP(DepthOfFieldEffect, FocusDistance, depth, FocusDistance);
    MAP(DepthOfFieldEffect, InFocusRadius, depth, InFocusRadius);
    MAP(DepthOfFieldEffect, NearIntensity, depth, NearIntensity);
    MAP(DepthOfFieldEffect, Enabled, depth, Enabled);

    // ===== SunRaysEffect (C++: sunray) =====
    MAP(SunRaysEffect, Intensity, sunray, Intensity);
    MAP(SunRaysEffect, Spread, sunray, Spread);
    MAP(SunRaysEffect, Enabled, sunray, Enabled);

    // ===== ProximityPrompt (C++: prompt) =====
    MAP(ProximityPrompt, ActionText, prompt, ActionText);
    MAP(ProximityPrompt, ObjectText, prompt, ObjectText);
    MAP(ProximityPrompt, HoldDuration, prompt, HoldDuration);
    MAP(ProximityPrompt, MaxActivationDistance, prompt, MaxActivationDistance);
    MAP(ProximityPrompt, KeyCode, prompt, KeyCode);
    MAP(ProximityPrompt, GamepadKeyCode, prompt, GamepadKeyCode);
    MAP(ProximityPrompt, Enabled, prompt, Enabled);
    MAP(ProximityPrompt, RequiresLineOfSight, prompt, RequiresLineOfSight);

    // ===== ClickDetector (C++: click) =====
    MAP(ClickDetector, MaxActivationDistance, click, MaxActivationDistance);
    MAP(ClickDetector, MouseIcon, click, MouseIcon);

    // ===== DragDetector (C++: drag) =====
    MAP(DragDetector, ReferenceInstance, drag, ReferenceInstance);
    MAP(DragDetector, MaxActivationDistance, drag, MaxActivationDistance);
    MAP(DragDetector, MaxDragAngle, drag, MaxDragAngle);
    MAP(DragDetector, MaxDragTranslation, drag, MaxDragTranslation);
    MAP(DragDetector, MinDragAngle, drag, MinDragAngle);
    MAP(DragDetector, MinDragTranslation, drag, MinDragTranslation);
    MAP(DragDetector, ActivatedCursorIcon, drag, ActivatedCursorIcon);
    MAP(DragDetector, CursorIcon, drag, CursorIcon);
    MAP(DragDetector, MaxForce, drag, MaxForce);
    MAP(DragDetector, MaxTorque, drag, MaxTorque);
    MAP(DragDetector, Responsiveness, drag, Responsiveness);

    // ===== MaterialColors (C++: material) =====
    MAP(MaterialColors, Asphalt, material, Asphalt);
    MAP(MaterialColors, Basalt, material, Basalt);
    MAP(MaterialColors, Brick, material, Brick);
    MAP(MaterialColors, Cobblestone, material, Cobblestone);
    MAP(MaterialColors, Concrete, material, Concrete);
    MAP(MaterialColors, CrackedLava, material, CrackedLava);
    MAP(MaterialColors, Glacier, material, Glacier);
    MAP(MaterialColors, Grass, material, Grass);
    MAP(MaterialColors, Ground, material, Ground);
    MAP(MaterialColors, Ice, material, Ice);
    MAP(MaterialColors, LeafyGrass, material, LeafyGrass);
    MAP(MaterialColors, Limestone, material, Limestone);
    MAP(MaterialColors, Mud, material, Mud);
    MAP(MaterialColors, Pavement, material, Pavement);
    MAP(MaterialColors, Rock, material, Rock);
    MAP(MaterialColors, Salt, material, Salt);
    MAP(MaterialColors, Sand, material, Sand);
    MAP(MaterialColors, Sandstone, material, Sandstone);
    MAP(MaterialColors, Slate, material, Slate);
    MAP(MaterialColors, Snow, material, Snow);
    MAP(MaterialColors, WoodPlanks, material, WoodPlanks);

    // ===== MeshContentProvider (C++: meshprovider) =====
    MAP(MeshContentProvider, AssetID, meshprovider, AssetID);
    MAP(MeshContentProvider, Cache, meshprovider, cache);
    MAP(MeshContentProvider, LRUCache, meshprovider, LRUCache);
    MAP(MeshContentProvider, MeshData, meshprovider, meshdata);
    MAP(MeshContentProvider, ToMeshData, meshprovider, ToMeshData);

    // ===== ParticleEmitter (C++: particle) =====
    MAP(ParticleEmitter, Acceleration, particle, Acceleration);
    MAP(ParticleEmitter, Brightness, particle, Brightness);
    MAP(ParticleEmitter, Drag, particle, Drag);
    MAP(ParticleEmitter, Lifetime, particle, Lifetime);
    MAP(ParticleEmitter, LightEmission, particle, LightEmission);
    MAP(ParticleEmitter, LightInfluence, particle, LightInfluence);
    MAP(ParticleEmitter, Rate, particle, Rate);
    MAP(ParticleEmitter, RotSpeed, particle, RotSpeed);
    MAP(ParticleEmitter, Rotation, particle, Rotation);
    MAP(ParticleEmitter, Speed, particle, Speed);
    MAP(ParticleEmitter, SpreadAngle, particle, SpreadAngle);
    MAP(ParticleEmitter, Texture, particle, Texture);
    MAP(ParticleEmitter, TimeScale, particle, TimeScale);
    MAP(ParticleEmitter, VelocityInheritance, particle, VelocityInheritance);
    MAP(ParticleEmitter, ZOffset, particle, ZOffset);

    // ===== SpawnLocation (C++: spawn) =====
    MAP(SpawnLocation, AllowTeamChangeOnTouch, spawn, AllowTeamChangeOnTouch);
    MAP(SpawnLocation, Enabled, spawn, Enabled);
    MAP(SpawnLocation, ForcefieldDuration, spawn, ForcefieldDuration);
    MAP(SpawnLocation, Neutral, spawn, Neutral);
    MAP(SpawnLocation, TeamColor, spawn, TeamColor);

    // ===== SurfaceAppearance (C++: surface) =====
    MAP(SurfaceAppearance, AlphaMode, surface, AlphaMode);
    MAP(SurfaceAppearance, Color, surface, tocolor);
    MAP(SurfaceAppearance, ColorMap, surface, ColorMap);
    MAP(SurfaceAppearance, EmissiveMaskContent, surface, EmissiveMaskContent);
    MAP(SurfaceAppearance, EmissiveStrength, surface, EmissiveStrength);
    MAP(SurfaceAppearance, EmissiveTint, surface, EmissiveTint);
    MAP(SurfaceAppearance, MetalnessMap, surface, MetalnessMap);
    MAP(SurfaceAppearance, NormalMap, surface, NormalMap);
    MAP(SurfaceAppearance, RoughnessMap, surface, RoughnessMap);

    // ===== StatsItem (C++: stat) =====
    MAP(StatsItem, Value, stat, Value);

    // ===== VehicleSeat (C++: vehicle) =====
    MAP(VehicleSeat, MaxSpeed, vehicle, MaxSpeed);
    MAP(VehicleSeat, SteerFloat, vehicle, SteerFloat);
    MAP(VehicleSeat, ThrottleFloat, vehicle, ThrottleFloat);
    MAP(VehicleSeat, Torque, vehicle, Torque);
    MAP(VehicleSeat, TurnSpeed, vehicle, TurnSpeed);

    // ===== Textures (C++: texture) — NOT TRY, endpoint class is "Textures" not "Texture" =====
    MAP(Textures, Decal_Texture, texture, Decal_Texture);
    MAP(Textures, Texture_Texture, texture, Texture_Texture);

    // ===== UnionOperation (C++: unionop) =====
    MAP(UnionOperation, AssetId, unionop, AssetId);

    // ===== CharacterMesh (C++: charmesh) =====
    MAP(CharacterMesh, BaseTextureId, charmesh, BaseTextureId);
    MAP(CharacterMesh, BodyPart, charmesh, BodyPart);
    MAP(CharacterMesh, MeshId, charmesh, MeshId);
    MAP(CharacterMesh, OverlayTextureId, charmesh, OverlayTextureId);

    // ===== Atmosphere field mismatch: endpoint uses "Color" not "tocolor" =====
    MAP(Atmosphere, Color, atmosphere, tocolor);
    TRY(atmosphere, Decay);
    TRY(atmosphere, Density);
    TRY(atmosphere, Glare);
    TRY(atmosphere, Haze);
    TRY(atmosphere, Offset);

    // ===== Namespaces that match endpoint (case-insensitive) =====
    TRY(animator, ActiveAnimations);
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
    TRY(clothing, Color3);
    TRY(clothing, Template);
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
    TRY(localscript, GUID);
    TRY(localscript, Hash);
    TRY(localscript, bytecode);
    TRY(meshdata, FaceEnd);
    TRY(meshdata, FaceStart);
    TRY(meshdata, VertexEnd);
    TRY(meshdata, VertexStart);
    TRY(meshpart, MeshId);
    TRY(meshpart, Texture);
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
    // ===== RunService (C++: run) — NOT TRY, endpoint class is "RunService" not "run" =====
    MAP(RunService, HeartbeatFPS, run, HeartbeatFPS);
    MAP(RunService, HeartbeatTask, run, HeartbeatTask);
    TRY(script, GUID);
    TRY(script, Hash);
    TRY(script, bytecode);
    TRY(scriptcontext, RequireBypass);
    TRY(seat, Occupant);
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
    TRY(specialmesh, MeshId);
    TRY(specialmesh, Scale);
    TRY(team, BrickColor);
    TRY(terrain, GrassLength);
    TRY(terrain, material);
    TRY(terrain, WaterColor);
    TRY(terrain, WaterReflectance);
    TRY(terrain, WaterTransparency);
    TRY(terrain, WaterWaveSize);
    TRY(terrain, WaterWaveSpeed);
    TRY(tool, CanBeDropped);
    TRY(tool, Enabled);
    TRY(tool, Grip);
    TRY(tool, ManualActivationOnly);
    TRY(tool, RequiresHandle);
    TRY(tool, TextureId);
    TRY(tool, Tooltip);
    TRY(value, ToValue);
    TRY(workspace, CurrentCamera);
    TRY(workspace, DistributedGameTime);
    TRY(workspace, ReadOnlyGravity);
    TRY(workspace, world);
    TRY(world, FallenPartsDestroyHeight);
    TRY(world, Gravity);
    TRY(world, GravityOverride);
    TRY(world, Primitives);
    TRY(world, worldStepsPerSec);
    MAP(World, AirProperties, world, air);

    // ===== Not in endpoint: silent, flag — use hardcoded defaults =====
    // silent: custom offsets, not part of standard Roblox dumper
    // flag: custom pointer chain, not part of standard Roblox dumper

#undef MAP
#undef TRY
}

}
