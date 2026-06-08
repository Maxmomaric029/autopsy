#pragma once

#include <cstdint>
#include <string>

namespace Offsets {
    extern bool Load();                           // call at startup
    extern uintptr_t Get(const char* cls, const char* field); // returns 0 if not found
}

namespace offset {
    inline std::string ClientVersion = "version-2b1721d47abf49aa";

    // Call this AFTER Offsets::Load() to override hardcoded defaults with server values
    inline void init_dynamic_offsets()
    {
#define LOAD_OFFSET(ns, field, cls, prop) \
        do { auto _v = Offsets::Get(cls, prop); if (_v) ns::field = _v; } while(0)

        LOAD_OFFSET(air, AirDensity, "Atmosphere", "AirDensity");
        LOAD_OFFSET(air, GlobalWind, "Atmosphere", "GlobalWind");
        LOAD_OFFSET(animation, Animation, "Animation", "Animation");
        LOAD_OFFSET(animation, animator, "Animation", "Animator");
        LOAD_OFFSET(animation, IsPlaying, "Animation", "IsPlaying");
        LOAD_OFFSET(animation, Looped, "Animation", "Looped");
        LOAD_OFFSET(animation, Speed, "Animation", "Speed");
        LOAD_OFFSET(animation, TimePosition, "Animation", "TimePosition");
        LOAD_OFFSET(animator, ActiveAnimations, "Animator", "ActiveAnimations");
        LOAD_OFFSET(atmosphere, tocolor, "Atmosphere", "Color");
        LOAD_OFFSET(atmosphere, Decay, "Atmosphere", "Decay");
        LOAD_OFFSET(atmosphere, Density, "Atmosphere", "Density");
        LOAD_OFFSET(atmosphere, Glare, "Atmosphere", "Glare");
        LOAD_OFFSET(atmosphere, Haze, "Atmosphere", "Haze");
        LOAD_OFFSET(atmosphere, Offset, "Atmosphere", "Offset");
        LOAD_OFFSET(attachment, Position, "Attachment", "Position");
        LOAD_OFFSET(basepart, CastShadow, "BasePart", "CastShadow");
        LOAD_OFFSET(basepart, Color3, "BasePart", "Color3");
        LOAD_OFFSET(basepart, Locked, "BasePart", "Locked");
        LOAD_OFFSET(basepart, Massless, "BasePart", "Massless");
        LOAD_OFFSET(basepart, primitive, "BasePart", "Primitive");
        LOAD_OFFSET(basepart, Reflectance, "BasePart", "Reflectance");
        LOAD_OFFSET(basepart, Shape, "BasePart", "Shape");
        LOAD_OFFSET(basepart, Transparency, "BasePart", "Transparency");
        LOAD_OFFSET(beam, Attachment0, "Beam", "Attachment0");
        LOAD_OFFSET(beam, Attachment1, "Beam", "Attachment1");
        LOAD_OFFSET(beam, Brightness, "Beam", "Brightness");
        LOAD_OFFSET(beam, CurveSize0, "Beam", "CurveSize0");
        LOAD_OFFSET(beam, CurveSize1, "Beam", "CurveSize1");
        LOAD_OFFSET(beam, LightEmission, "Beam", "LightEmission");
        LOAD_OFFSET(beam, LightInfluence, "Beam", "LightInfluence");
        LOAD_OFFSET(beam, Texture, "Beam", "Texture");
        LOAD_OFFSET(beam, TextureLength, "Beam", "TextureLength");
        LOAD_OFFSET(beam, TextureSpeed, "Beam", "TextureSpeed");
        LOAD_OFFSET(beam, Width0, "Beam", "Width0");
        LOAD_OFFSET(beam, Width1, "Beam", "Width1");
        LOAD_OFFSET(beam, ZOffset, "Beam", "ZOffset");
        LOAD_OFFSET(bloom, Enabled, "BloomEffect", "Enabled");
        LOAD_OFFSET(bloom, Intensity, "BloomEffect", "Intensity");
        LOAD_OFFSET(bloom, Size, "BloomEffect", "Size");
        LOAD_OFFSET(bloom, Threshold, "BloomEffect", "Threshold");
        LOAD_OFFSET(blur, Enabled, "BlurEffect", "Enabled");
        LOAD_OFFSET(blur, Size, "BlurEffect", "Size");
        LOAD_OFFSET(bytecode, Pointer, "LocalScript", "BytecodePointer");
        LOAD_OFFSET(bytecode, Size, "LocalScript", "BytecodeSize");
        LOAD_OFFSET(camera, CameraSubject, "Camera", "CameraSubject");
        LOAD_OFFSET(camera, CameraType, "Camera", "CameraType");
        LOAD_OFFSET(camera, FieldOfView, "Camera", "FieldOfView");
        LOAD_OFFSET(camera, ImagePlaneDepth, "Camera", "ImagePlaneDepth");
        LOAD_OFFSET(camera, Position, "Camera", "Position");
        LOAD_OFFSET(camera, Rotation, "Camera", "Rotation");
        LOAD_OFFSET(camera, Viewport, "Camera", "Viewport");
        LOAD_OFFSET(camera, ViewportSize, "Camera", "ViewportSize");
        LOAD_OFFSET(charmesh, BaseTextureId, "CharacterMesh", "BaseTextureId");
        LOAD_OFFSET(charmesh, BodyPart, "CharacterMesh", "BodyPart");
        LOAD_OFFSET(charmesh, MeshId, "CharacterMesh", "MeshId");
        LOAD_OFFSET(charmesh, OverlayTextureId, "CharacterMesh", "OverlayTextureId");
        LOAD_OFFSET(click, MaxActivationDistance, "ClickDetector", "MaxActivationDistance");
        LOAD_OFFSET(click, MouseIcon, "ClickDetector", "MouseIcon");
        LOAD_OFFSET(clothing, Color3, "Clothing", "Color3");
        LOAD_OFFSET(clothing, Template, "Clothing", "Template");
        LOAD_OFFSET(correction, Brightness, "ColorCorrectionEffect", "Brightness");
        LOAD_OFFSET(correction, Contrast, "ColorCorrectionEffect", "Contrast");
        LOAD_OFFSET(correction, Enabled, "ColorCorrectionEffect", "Enabled");
        LOAD_OFFSET(correction, TintColor, "ColorCorrectionEffect", "TintColor");
        LOAD_OFFSET(grading, Enabled, "ColorGradingEffect", "Enabled");
        LOAD_OFFSET(grading, TonemapperPreset, "ColorGradingEffect", "TonemapperPreset");
        LOAD_OFFSET(datamodel, CreatorId, "DataModel", "CreatorId");
        LOAD_OFFSET(datamodel, GameId, "DataModel", "GameId");
        LOAD_OFFSET(datamodel, GameLoaded, "DataModel", "GameLoaded");
        LOAD_OFFSET(datamodel, JobId, "DataModel", "JobId");
        LOAD_OFFSET(datamodel, PlaceId, "DataModel", "PlaceId");
        LOAD_OFFSET(datamodel, PlaceVersion, "DataModel", "PlaceVersion");
        LOAD_OFFSET(datamodel, PrimitiveCount, "DataModel", "PrimitiveCount");
        LOAD_OFFSET(datamodel, scriptcontext, "DataModel", "ScriptContext");
        LOAD_OFFSET(datamodel, ServerIP, "DataModel", "ServerIP");
        LOAD_OFFSET(datamodel, ToRenderView1, "DataModel", "ToRenderView1");
        LOAD_OFFSET(datamodel, ToRenderView2, "DataModel", "ToRenderView2");
        LOAD_OFFSET(datamodel, ToRenderView3, "DataModel", "ToRenderView3");
        LOAD_OFFSET(datamodel, workspace, "DataModel", "WorkspacePointer");
        LOAD_OFFSET(depth, Enabled, "DepthOfFieldEffect", "Enabled");
        LOAD_OFFSET(depth, FarIntensity, "DepthOfFieldEffect", "FarIntensity");
        LOAD_OFFSET(depth, FocusDistance, "DepthOfFieldEffect", "FocusDistance");
        LOAD_OFFSET(depth, InFocusRadius, "DepthOfFieldEffect", "InFocusRadius");
        LOAD_OFFSET(depth, NearIntensity, "DepthOfFieldEffect", "NearIntensity");
        LOAD_OFFSET(drag, ActivatedCursorIcon, "DragDetector", "ActivatedCursorIcon");
        LOAD_OFFSET(drag, CursorIcon, "DragDetector", "CursorIcon");
        LOAD_OFFSET(drag, MaxActivationDistance, "DragDetector", "MaxActivationDistance");
        LOAD_OFFSET(fakemodel, Pointer, "FakeModel", "Pointer");
        LOAD_OFFSET(fakemodel, RealDataModel, "FakeModel", "RealDataModel");
        LOAD_OFFSET(gui2d, AbsolutePosition, "GuiObject", "AbsolutePosition");
        LOAD_OFFSET(gui2d, AbsoluteRotation, "GuiObject", "AbsoluteRotation");
        LOAD_OFFSET(gui2d, AbsoluteSize, "GuiObject", "AbsoluteSize");
        LOAD_OFFSET(gui, BackgroundColor3, "Frame", "BackgroundColor3");
        LOAD_OFFSET(gui, BackgroundTransparency, "Frame", "BackgroundTransparency");
        LOAD_OFFSET(gui, Image, "ImageLabel", "Image");
        LOAD_OFFSET(gui, LayoutOrder, "GuiObject", "LayoutOrder");
        LOAD_OFFSET(gui, Position, "GuiObject", "Position");
        LOAD_OFFSET(gui, RichText, "TextLabel", "RichText");
        LOAD_OFFSET(gui, Rotation, "GuiObject", "Rotation");
        LOAD_OFFSET(gui, ScreenGui_Enabled, "ScreenGui", "Enabled");
        LOAD_OFFSET(gui, Size, "GuiObject", "Size");
        LOAD_OFFSET(gui, text, "TextLabel", "Text");
        LOAD_OFFSET(gui, TextColor3, "TextLabel", "TextColor3");
        LOAD_OFFSET(gui, Visible, "GuiObject", "Visible");
        LOAD_OFFSET(gui, ZIndex, "GuiObject", "ZIndex");
        LOAD_OFFSET(humanoid, AutoJumpEnabled, "Humanoid", "AutoJumpEnabled");
        LOAD_OFFSET(humanoid, AutoRotate, "Humanoid", "AutoRotate");
        LOAD_OFFSET(humanoid, AutomaticScalingEnabled, "Humanoid", "AutomaticScalingEnabled");
        LOAD_OFFSET(humanoid, BreakJointsOnDeath, "Humanoid", "BreakJointsOnDeath");
        LOAD_OFFSET(humanoid, CameraOffset, "Humanoid", "CameraOffset");
        LOAD_OFFSET(humanoid, DisplayDistanceType, "Humanoid", "DisplayDistanceType");
        LOAD_OFFSET(humanoid, DisplayName, "Humanoid", "DisplayName");
        LOAD_OFFSET(humanoid, EvaluateStateMachine, "Humanoid", "EvaluateStateMachine");
        LOAD_OFFSET(humanoid, FloorMaterial, "Humanoid", "FloorMaterial");
        LOAD_OFFSET(humanoid, Health, "Humanoid", "Health");
        LOAD_OFFSET(humanoid, HealthDisplayDistance, "Humanoid", "HealthDisplayDistance");
        LOAD_OFFSET(humanoid, HealthDisplayType, "Humanoid", "HealthDisplayType");
        LOAD_OFFSET(humanoid, HipHeight, "Humanoid", "HipHeight");
        LOAD_OFFSET(humanoid, HumanoidRootPart, "Humanoid", "HumanoidRootPart");
        LOAD_OFFSET(humanoid, HumanoidState, "Humanoid", "HumanoidState");
        LOAD_OFFSET(humanoid, HumanoidStateID, "Humanoid", "HumanoidStateID");
        LOAD_OFFSET(humanoid, IsWalking, "Humanoid", "IsWalking");
        LOAD_OFFSET(humanoid, Jump, "Humanoid", "Jump");
        LOAD_OFFSET(humanoid, JumpHeight, "Humanoid", "JumpHeight");
        LOAD_OFFSET(humanoid, JumpPower, "Humanoid", "JumpPower");
        LOAD_OFFSET(humanoid, MaxHealth, "Humanoid", "MaxHealth");
        LOAD_OFFSET(humanoid, MaxSlopeAngle, "Humanoid", "MaxSlopeAngle");
        LOAD_OFFSET(humanoid, MoveDirection, "Humanoid", "MoveDirection");
        LOAD_OFFSET(humanoid, MoveToPart, "Humanoid", "MoveToPart");
        LOAD_OFFSET(humanoid, MoveToPoint, "Humanoid", "MoveToPoint");
        LOAD_OFFSET(humanoid, NameDisplayDistance, "Humanoid", "NameDisplayDistance");
        LOAD_OFFSET(humanoid, NameOcclusion, "Humanoid", "NameOcclusion");
        LOAD_OFFSET(humanoid, PlatformStand, "Humanoid", "PlatformStand");
        LOAD_OFFSET(humanoid, RequiresNeck, "Humanoid", "RequiresNeck");
        LOAD_OFFSET(humanoid, RigType, "Humanoid", "RigType");
        LOAD_OFFSET(humanoid, SeatPart, "Humanoid", "SeatPart");
        LOAD_OFFSET(humanoid, Sit, "Humanoid", "Sit");
        LOAD_OFFSET(humanoid, TargetPoint, "Humanoid", "TargetPoint");
        LOAD_OFFSET(humanoid, UseJumpPower, "Humanoid", "UseJumpPower");
        LOAD_OFFSET(humanoid, WalkTimer, "Humanoid", "WalkTimer");
        LOAD_OFFSET(humanoid, walkspeed, "Humanoid", "WalkSpeed");
        LOAD_OFFSET(humanoid, WalkspeedCheck, "Humanoid", "WalkSpeedCheck");
        LOAD_OFFSET(instance, AttributeContainer, "Instance", "AttributeContainer");
        LOAD_OFFSET(instance, AttributeList, "Instance", "AttributeList");
        LOAD_OFFSET(instance, AttributeToNext, "Instance", "AttributeToNext");
        LOAD_OFFSET(instance, AttributeToValue, "Instance", "AttributeToValue");
        LOAD_OFFSET(instance, ChildrenEnd, "Instance", "ChildrenEndOffset");
        LOAD_OFFSET(instance, ChildrenStart, "Instance", "ChildrenStart");
        LOAD_OFFSET(instance, ClassBase, "Instance", "ClassBase");
        LOAD_OFFSET(instance, ClassDescriptor, "Instance", "ClassDescriptor");
        LOAD_OFFSET(instance, ClassName, "Instance", "ClassName");
        LOAD_OFFSET(instance, name, "Instance", "Name");
        LOAD_OFFSET(instance, parent, "Instance", "Parent");
        LOAD_OFFSET(instance, This, "Instance", "ThisPointer");
        LOAD_OFFSET(light, Ambient, "Lighting", "Ambient");
        LOAD_OFFSET(light, Brightness, "Lighting", "Brightness");
        LOAD_OFFSET(light, ClockTime, "Lighting", "ClockTime");
        LOAD_OFFSET(light, ColorShift_Bottom, "Lighting", "ColorShift_Bottom");
        LOAD_OFFSET(light, ColorShift_Top, "Lighting", "ColorShift_Top");
        LOAD_OFFSET(light, EnvironmentDiffuseScale, "Lighting", "EnvironmentDiffuseScale");
        LOAD_OFFSET(light, EnvironmentSpecularScale, "Lighting", "EnvironmentSpecularScale");
        LOAD_OFFSET(light, ExposureCompensation, "Lighting", "ExposureCompensation");
        LOAD_OFFSET(light, FogColor, "Lighting", "FogColor");
        LOAD_OFFSET(light, FogEnd, "Lighting", "FogEnd");
        LOAD_OFFSET(light, FogStart, "Lighting", "FogStart");
        LOAD_OFFSET(light, GeographicLatitude, "Lighting", "GeographicLatitude");
        LOAD_OFFSET(light, GlobalShadows, "Lighting", "GlobalShadows");
        LOAD_OFFSET(light, GradientBottom, "Lighting", "GradientBottom");
        LOAD_OFFSET(light, GradientTop, "Lighting", "GradientTop");
        LOAD_OFFSET(light, LightColor, "Lighting", "LightColor");
        LOAD_OFFSET(light, LightDirection, "Lighting", "LightDirection");
        LOAD_OFFSET(light, MoonPosition, "Lighting", "MoonPosition");
        LOAD_OFFSET(light, OutdoorAmbient, "Lighting", "OutdoorAmbient");
        LOAD_OFFSET(light, sky, "Lighting", "Sky");
        LOAD_OFFSET(light, Source, "Lighting", "Source");
        LOAD_OFFSET(light, SunPosition, "Lighting", "SunPosition");
        LOAD_OFFSET(localscript, bytecode, "LocalScript", "Bytecode");
        LOAD_OFFSET(localscript, GUID, "LocalScript", "GUID");
        LOAD_OFFSET(localscript, Hash, "LocalScript", "Hash");
        LOAD_OFFSET(meshprovider, AssetID, "MeshProvider", "AssetID");
        LOAD_OFFSET(meshprovider, cache, "MeshProvider", "Cache");
        LOAD_OFFSET(meshprovider, LRUCache, "MeshProvider", "LRUCache");
        LOAD_OFFSET(meshprovider, meshdata, "MeshProvider", "MeshData");
        LOAD_OFFSET(meshdata, FaceEnd, "MeshData", "FaceEnd");
        LOAD_OFFSET(meshdata, FaceStart, "MeshData", "FaceStart");
        LOAD_OFFSET(meshdata, VertexEnd, "MeshData", "VertexEnd");
        LOAD_OFFSET(meshdata, VertexStart, "MeshData", "VertexStart");
        LOAD_OFFSET(meshpart, MeshId, "MeshPart", "MeshId");
        LOAD_OFFSET(meshpart, Texture, "MeshPart", "Texture");
        LOAD_OFFSET(misc, Adornee, "ObjectValue", "Adornee");
        LOAD_OFFSET(misc, AnimationId, "Animation", "AnimationId");
        LOAD_OFFSET(misc, StringLength, "RBXString", "Length");
        LOAD_OFFSET(misc, Value, "ValueBase", "Value");
        LOAD_OFFSET(model, PrimaryPart, "Model", "PrimaryPart");
        LOAD_OFFSET(model, Scale, "Model", "Scale");
        LOAD_OFFSET(modulescript, bytecode, "ModuleScript", "Bytecode");
        LOAD_OFFSET(modulescript, GUID, "ModuleScript", "GUID");
        LOAD_OFFSET(modulescript, Hash, "ModuleScript", "Hash");
        LOAD_OFFSET(mouseservice, InputObject, "MouseService", "InputObject");
        LOAD_OFFSET(mouseservice, InputObject2, "MouseService", "InputObject2");
        LOAD_OFFSET(mouseservice, MousePosition, "MouseService", "MousePosition");
        LOAD_OFFSET(mouseservice, SensitivityPointer, "MouseService", "SensitivityPointer");
        LOAD_OFFSET(particle, Acceleration, "ParticleEmitter", "Acceleration");
        LOAD_OFFSET(particle, Brightness, "ParticleEmitter", "Brightness");
        LOAD_OFFSET(particle, Drag, "ParticleEmitter", "Drag");
        LOAD_OFFSET(particle, Lifetime, "ParticleEmitter", "Lifetime");
        LOAD_OFFSET(particle, LightEmission, "ParticleEmitter", "LightEmission");
        LOAD_OFFSET(particle, LightInfluence, "ParticleEmitter", "LightInfluence");
        LOAD_OFFSET(particle, Rate, "ParticleEmitter", "Rate");
        LOAD_OFFSET(particle, RotSpeed, "ParticleEmitter", "RotSpeed");
        LOAD_OFFSET(particle, Rotation, "ParticleEmitter", "Rotation");
        LOAD_OFFSET(particle, Speed, "ParticleEmitter", "Speed");
        LOAD_OFFSET(particle, SpreadAngle, "ParticleEmitter", "SpreadAngle");
        LOAD_OFFSET(particle, Texture, "ParticleEmitter", "Texture");
        LOAD_OFFSET(particle, TimeScale, "ParticleEmitter", "TimeScale");
        LOAD_OFFSET(particle, VelocityInheritance, "ParticleEmitter", "VelocityInheritance");
        LOAD_OFFSET(particle, ZOffset, "ParticleEmitter", "ZOffset");
        LOAD_OFFSET(player, AccountAge, "Player", "AccountAge");
        LOAD_OFFSET(player, CameraMode, "Player", "CameraMode");
        LOAD_OFFSET(player, DisplayName, "Player", "DisplayName");
        LOAD_OFFSET(player, HealthDisplayDistance, "Player", "HealthDisplayDistance");
        LOAD_OFFSET(player, LocalPlayer, "Player", "LocalPlayer");
        LOAD_OFFSET(player, LocaleId, "Player", "LocaleId");
        LOAD_OFFSET(player, MaxZoomDistance, "Player", "MaxZoomDistance");
        LOAD_OFFSET(player, MinZoomDistance, "Player", "MinZoomDistance");
        LOAD_OFFSET(player, ModelInstance, "Player", "ModelInstance");
        LOAD_OFFSET(player, mouse, "Player", "Mouse");
        LOAD_OFFSET(player, NameDisplayDistance, "Player", "NameDisplayDistance");
        LOAD_OFFSET(player, team, "Player", "Team");
        LOAD_OFFSET(player, TeamColor, "Player", "TeamColor");
        LOAD_OFFSET(player, UserId, "Player", "UserId");
        LOAD_OFFSET(playermouse, icon, "PlayerMouse", "Icon");
        LOAD_OFFSET(playermouse, workspace, "PlayerMouse", "WorkspacePointer");
        LOAD_OFFSET(primitive, AssemblyAngularVelocity, "Primitive", "AssemblyAngularVelocity");
        LOAD_OFFSET(primitive, AssemblyLinearVelocity, "Primitive", "AssemblyLinearVelocity");
        LOAD_OFFSET(primitive, Flags, "Primitive", "Flags");
        LOAD_OFFSET(primitive, Owner, "Primitive", "Owner");
        LOAD_OFFSET(primitive, Position, "Primitive", "Position");
        LOAD_OFFSET(primitive, Rotation, "Primitive", "Rotation");
        LOAD_OFFSET(primitive, Size, "Primitive", "Size");
        LOAD_OFFSET(primitive, Validate, "Primitive", "Validate");
        LOAD_OFFSET(primitiveflag, Anchored, "Primitive", "AnchoredFlag");
        LOAD_OFFSET(primitiveflag, CanCollide, "Primitive", "CanCollideFlag");
        LOAD_OFFSET(primitiveflag, CanQuery, "Primitive", "CanQueryFlag");
        LOAD_OFFSET(primitiveflag, CanTouch, "Primitive", "CanTouchFlag");
        LOAD_OFFSET(prompt, ActionText, "ProximityPrompt", "ActionText");
        LOAD_OFFSET(prompt, Enabled, "ProximityPrompt", "Enabled");
        LOAD_OFFSET(prompt, GamepadKeyCode, "ProximityPrompt", "GamepadKeyCode");
        LOAD_OFFSET(prompt, HoldDuration, "ProximityPrompt", "HoldDuration");
        LOAD_OFFSET(prompt, KeyCode, "ProximityPrompt", "KeyCode");
        LOAD_OFFSET(prompt, MaxActivationDistance, "ProximityPrompt", "MaxActivationDistance");
        LOAD_OFFSET(prompt, ObjectText, "ProximityPrompt", "ObjectText");
        LOAD_OFFSET(prompt, RequiresLineOfSight, "ProximityPrompt", "RequiresLineOfSight");
        LOAD_OFFSET(job, fakemodel, "Job", "FakeModel");
        LOAD_OFFSET(job, RealDataModel, "Job", "RealDataModel");
        LOAD_OFFSET(job, view, "Job", "View");
        LOAD_OFFSET(view, DeviceD3D11, "RenderView", "DeviceD3D11");
        LOAD_OFFSET(view, LightingValid, "RenderView", "LightingValid");
        LOAD_OFFSET(view, SkyValid, "RenderView", "SkyValid");
        LOAD_OFFSET(view, render, "RenderView", "RenderPointer");
        LOAD_OFFSET(run, HeartbeatFPS, "RunService", "HeartbeatFPS");
        LOAD_OFFSET(run, HeartbeatTask, "RunService", "HeartbeatTask");
        LOAD_OFFSET(script, bytecode, "Script", "Bytecode");
        LOAD_OFFSET(script, GUID, "Script", "GUID");
        LOAD_OFFSET(script, Hash, "Script", "Hash");
        LOAD_OFFSET(seat, Occupant, "Seat", "Occupant");
        LOAD_OFFSET(sky, MoonAngularSize, "Sky", "MoonAngularSize");
        LOAD_OFFSET(sky, MoonTextureId, "Sky", "MoonTextureId");
        LOAD_OFFSET(sky, SkyboxBk, "Sky", "SkyboxBk");
        LOAD_OFFSET(sky, SkyboxDn, "Sky", "SkyboxDn");
        LOAD_OFFSET(sky, SkyboxFt, "Sky", "SkyboxFt");
        LOAD_OFFSET(sky, SkyboxLf, "Sky", "SkyboxLf");
        LOAD_OFFSET(sky, SkyboxOrientation, "Sky", "SkyboxOrientation");
        LOAD_OFFSET(sky, SkyboxRt, "Sky", "SkyboxRt");
        LOAD_OFFSET(sky, SkyboxUp, "Sky", "SkyboxUp");
        LOAD_OFFSET(sky, StarCount, "Sky", "StarCount");
        LOAD_OFFSET(sky, SunAngularSize, "Sky", "SunAngularSize");
        LOAD_OFFSET(sky, SunTextureId, "Sky", "SunTextureId");
        LOAD_OFFSET(sound, Looped, "Sound", "Looped");
        LOAD_OFFSET(sound, PlaybackSpeed, "Sound", "PlaybackSpeed");
        LOAD_OFFSET(sound, Playing, "Sound", "Playing");
        LOAD_OFFSET(sound, RollOffMaxDistance, "Sound", "RollOffMaxDistance");
        LOAD_OFFSET(sound, RollOffMinDistance, "Sound", "RollOffMinDistance");
        LOAD_OFFSET(sound, SoundGroup, "Sound", "SoundGroup");
        LOAD_OFFSET(sound, SoundId, "Sound", "SoundId");
        LOAD_OFFSET(sound, Volume, "Sound", "Volume");
        LOAD_OFFSET(spawn, AllowTeamChangeOnTouch, "SpawnLocation", "AllowTeamChangeOnTouch");
        LOAD_OFFSET(spawn, Enabled, "SpawnLocation", "Enabled");
        LOAD_OFFSET(spawn, ForcefieldDuration, "SpawnLocation", "ForcefieldDuration");
        LOAD_OFFSET(spawn, Neutral, "SpawnLocation", "Neutral");
        LOAD_OFFSET(spawn, TeamColor, "SpawnLocation", "TeamColor");
        LOAD_OFFSET(specialmesh, MeshId, "SpecialMesh", "MeshId");
        LOAD_OFFSET(specialmesh, Scale, "SpecialMesh", "Scale");
        LOAD_OFFSET(stat, Value, "IntValue", "Value");
        LOAD_OFFSET(sunray, Enabled, "SunRaysEffect", "Enabled");
        LOAD_OFFSET(sunray, Intensity, "SunRaysEffect", "Intensity");
        LOAD_OFFSET(sunray, Spread, "SunRaysEffect", "Spread");
        LOAD_OFFSET(surface, AlphaMode, "SurfaceAppearance", "AlphaMode");
        LOAD_OFFSET(surface, tocolor, "SurfaceAppearance", "Color");
        LOAD_OFFSET(surface, ColorMap, "SurfaceAppearance", "ColorMap");
        LOAD_OFFSET(surface, EmissiveMaskContent, "SurfaceAppearance", "EmissiveMaskContent");
        LOAD_OFFSET(surface, EmissiveStrength, "SurfaceAppearance", "EmissiveStrength");
        LOAD_OFFSET(surface, EmissiveTint, "SurfaceAppearance", "EmissiveTint");
        LOAD_OFFSET(surface, MetalnessMap, "SurfaceAppearance", "MetalnessMap");
        LOAD_OFFSET(surface, NormalMap, "SurfaceAppearance", "NormalMap");
        LOAD_OFFSET(surface, RoughnessMap, "SurfaceAppearance", "RoughnessMap");
        LOAD_OFFSET(task, JobEnd, "TaskScheduler", "JobEnd");
        LOAD_OFFSET(task, JobName, "TaskScheduler", "JobName");
        LOAD_OFFSET(task, JobStart, "TaskScheduler", "JobStart");
        LOAD_OFFSET(task, MaxFPS, "TaskScheduler", "MaxFPS");
        LOAD_OFFSET(task, Pointer, "TaskScheduler", "Pointer");
        LOAD_OFFSET(team, BrickColor, "Team", "BrickColor");
        LOAD_OFFSET(terrain, GrassLength, "Terrain", "GrassLength");
        LOAD_OFFSET(terrain, material, "Terrain", "Material");
        LOAD_OFFSET(terrain, WaterColor, "Terrain", "WaterColor");
        LOAD_OFFSET(terrain, WaterReflectance, "Terrain", "WaterReflectance");
        LOAD_OFFSET(terrain, WaterTransparency, "Terrain", "WaterTransparency");
        LOAD_OFFSET(terrain, WaterWaveSize, "Terrain", "WaterWaveSize");
        LOAD_OFFSET(terrain, WaterWaveSpeed, "Terrain", "WaterWaveSpeed");
        LOAD_OFFSET(texture, Decal_Texture, "Decal", "Texture");
        LOAD_OFFSET(texture, Texture_Texture, "Texture", "Texture");
        LOAD_OFFSET(tool, CanBeDropped, "Tool", "CanBeDropped");
        LOAD_OFFSET(tool, Enabled, "Tool", "Enabled");
        LOAD_OFFSET(tool, Grip, "Tool", "Grip");
        LOAD_OFFSET(tool, ManualActivationOnly, "Tool", "ManualActivationOnly");
        LOAD_OFFSET(tool, RequiresHandle, "Tool", "RequiresHandle");
        LOAD_OFFSET(tool, TextureId, "Tool", "TextureId");
        LOAD_OFFSET(tool, Tooltip, "Tool", "Tooltip");
        LOAD_OFFSET(unionop, AssetId, "UnionOperation", "AssetId");
        LOAD_OFFSET(input, inputstate, "UserInputService", "InputState");
        LOAD_OFFSET(vehicle, MaxSpeed, "VehicleSeat", "MaxSpeed");
        LOAD_OFFSET(vehicle, SteerFloat, "VehicleSeat", "SteerFloat");
        LOAD_OFFSET(vehicle, ThrottleFloat, "VehicleSeat", "ThrottleFloat");
        LOAD_OFFSET(vehicle, Torque, "VehicleSeat", "Torque");
        LOAD_OFFSET(vehicle, TurnSpeed, "VehicleSeat", "TurnSpeed");
        LOAD_OFFSET(render, Dimensions, "RenderEngine", "Dimensions");
        LOAD_OFFSET(render, fakemodel, "RenderEngine", "FakeModel");
        LOAD_OFFSET(render, Pointer, "RenderEngine", "Pointer");
        LOAD_OFFSET(render, view, "RenderEngine", "ViewPointer");
        LOAD_OFFSET(render, ViewMatrix, "RenderEngine", "ViewMatrix");
        LOAD_OFFSET(weld, Part0, "Weld", "Part0");
        LOAD_OFFSET(weld, Part1, "Weld", "Part1");
        LOAD_OFFSET(weldconstraint, Part0, "WeldConstraint", "Part0");
        LOAD_OFFSET(weldconstraint, Part1, "WeldConstraint", "Part1");
        LOAD_OFFSET(inputstate, CapsLock, "InputState", "CapsLock");
        LOAD_OFFSET(inputstate, CurrentTextBox, "InputState", "CurrentTextBox");
        LOAD_OFFSET(workspace, CurrentCamera, "Workspace", "CurrentCamera");
        LOAD_OFFSET(workspace, DistributedGameTime, "Workspace", "DistributedGameTime");
        LOAD_OFFSET(workspace, ReadOnlyGravity, "Workspace", "ReadOnlyGravity");
        LOAD_OFFSET(workspace, world, "Workspace", "WorldPointer");
        LOAD_OFFSET(world, air, "World", "Air");
        LOAD_OFFSET(world, FallenPartsDestroyHeight, "World", "FallenPartsDestroyHeight");
        LOAD_OFFSET(world, Gravity, "World", "Gravity");
        LOAD_OFFSET(world, Primitives, "World", "Primitives");
        LOAD_OFFSET(world, worldStepsPerSec, "World", "WorldStepsPerSec");
        LOAD_OFFSET(world, GravityOverride, "World", "GravityOverride");
        LOAD_OFFSET(silent, FramePositionOffsetX, "Frame", "PositionOffsetX");
        LOAD_OFFSET(silent, FramePositionOffsetY, "Frame", "PositionOffsetY");
        LOAD_OFFSET(silent, FramePositionX, "Frame", "PositionX");
        LOAD_OFFSET(silent, FramePositionY, "Frame", "PositionY");
        LOAD_OFFSET(silent, FrameRotation, "Frame", "FrameRotation");
        LOAD_OFFSET(silent, FrameSizeOffsetX, "Frame", "SizeOffsetX");
        LOAD_OFFSET(silent, FrameSizeOffsetY, "Frame", "SizeOffsetY");
        LOAD_OFFSET(silent, FrameSizeX, "Frame", "SizeX");
        LOAD_OFFSET(silent, FrameSizeY, "Frame", "SizeY");
        LOAD_OFFSET(flag, List, "Flag", "List");
        LOAD_OFFSET(flag, ToValueGetSet, "Flag", "ToValueGetSet");
        LOAD_OFFSET(value, ToValue, "Value", "ToValue");

#undef LOAD_OFFSET
    }

    // ---- Hardcoded default offset values (fallbacks if Offsets::Get() returns 0) ----
    // These are defined as non-constexpr so init_dynamic_offsets() can override them

    namespace air {
         inline uintptr_t AirDensity = 0x18;
         inline uintptr_t GlobalWind = 0x3c;
    }

    namespace animation {
        inline uintptr_t Animation = 0xd0;
        inline uintptr_t animator = 0x118;
        inline uintptr_t IsPlaying = 0xa18;
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
        inline uintptr_t Pointer = 0x74f6758;
        inline uintptr_t RealDataModel = 0x1d0;
    }

    namespace gui2d {
        inline uintptr_t AbsolutePosition = 0x110;
        inline uintptr_t AbsoluteRotation = 0x188;
        inline uintptr_t AbsoluteSize = 0x118;
    }

    namespace gui {
        inline uintptr_t BackgroundColor3 = 0x548;
        inline uintptr_t BackgroundTransparency = 0x554;
        inline uintptr_t BorderColor3 = 0x554;
        inline uintptr_t Image = 0x990;
        inline uintptr_t LayoutOrder = 0x588;
        inline uintptr_t Position = 0x518;
        inline uintptr_t RichText = 0xb60;
        inline uintptr_t Rotation = 0x188;
        inline uintptr_t ScreenGui_Enabled = 0x4cc;
        inline uintptr_t Size = 0x538;
        inline uintptr_t text = 0xda8;
        inline uintptr_t TextColor3 = 0xe58;
        inline uintptr_t Visible = 0x5b5;
        inline uintptr_t ZIndex = 0x5ac;
    }

    namespace humanoid {
        inline uintptr_t AutoJumpEnabled = 0x1e0;
        inline uintptr_t AutoRotate = 0x1e1;
        inline uintptr_t AutomaticScalingEnabled = 0x1e2;
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
        inline uintptr_t HumanoidRootPart = 0x488;
        inline uintptr_t HumanoidState = 0x8a8;
        inline uintptr_t HumanoidStateID = 0x20;
        inline uintptr_t IsWalking = 0x927;
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
        inline uintptr_t RequiresNeck = 0x1e9;
        inline uintptr_t RigType = 0x1cc;
        inline uintptr_t SeatPart = 0x120;
        inline uintptr_t Sit = 0x1e9;
        inline uintptr_t TargetPoint = 0x164;
        inline uintptr_t UseJumpPower = 0x1ec;
        inline uintptr_t WalkTimer = 0x418;
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
        inline uintptr_t ClassBase = 0x47d8;
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
        inline uintptr_t cache = 0xf0;
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
        inline uintptr_t MeshId = 0x2f8;
        inline uintptr_t Texture = 0x328;
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
        inline uintptr_t mouse = 0x1178;
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
        inline uintptr_t AllowTeamChangeOnTouch = 0x45;
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
        inline uintptr_t Pointer = 0x7bfe988;
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
        inline uintptr_t AssetId = 0x2f0;
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

    namespace render {
        inline uintptr_t Dimensions = 0xaa0;
        inline uintptr_t fakemodel = 0xa80;
        inline uintptr_t Pointer = 0x7bd51f8;
        inline uintptr_t view = 0xb80;
        inline uintptr_t ViewMatrix = 0x140;
    }

    namespace weld {
        inline uintptr_t Part0 = 0x130;
        inline uintptr_t Part1 = 0x140;
    }

    namespace weldconstraint {
        inline uintptr_t Part0 = 0xd0;
        inline uintptr_t Part1 = 0xe0;
    }

    namespace inputstate {
        inline uintptr_t CapsLock = 0x40;
        inline uintptr_t CurrentTextBox = 0x48;
    }

    namespace workspace {
        inline uintptr_t CurrentCamera = 0x4b0;
        inline uintptr_t DistributedGameTime = 0x4d0;
        inline uintptr_t ReadOnlyGravity = 0x9e0;
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
}
