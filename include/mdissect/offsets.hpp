#ifndef MDISSECT_OFFSETS_HPP
#define MDISSECT_OFFSETS_HPP

#include <cstdint>

// offsets are taken from IDA

namespace offsets {
    // _MonoDomain
    constexpr int32_t MonoDomainDomainId = 0xBC;
    constexpr int32_t MonoDomainDomainAssemblies = 0xC8;

    // _MonoAssembly
    constexpr int32_t MonoAssemblyBaseDir = 0x8;
    constexpr int32_t MonoAssemblyName = 0x10;
    constexpr int32_t MonoAssemblyImage = 0x60;
    constexpr int32_t MonoAssemblyFriendAssemblyNames = 0x68;
    constexpr int32_t MonoAssemblyDynamic = 0x72;

    // _MonoImage
    constexpr int32_t MonoImageName = 0x20;
    constexpr int32_t MonoImageAssemblyName = 0x28;
    constexpr int32_t MonoImageModuleName = 0x30;
    constexpr int32_t MonoImageVersion = 0x38;
    constexpr int32_t MonoImageVersionMajor = 0x40;
    constexpr int32_t MonoImageVersionMinor = 0x42;
    constexpr int32_t MonoImageGuid = 0x48;
    constexpr int32_t MonoImageAssembly = 0x4B0;
    constexpr int32_t MonoImageClassCache = 0x4C0;

    // _MonoAssemblyName
    constexpr int32_t MonoAssemblyNameName = 0x0;
    constexpr int32_t MonoAssemblyNameMajor = 0x40;
    constexpr int32_t MonoAssemblyNameMinor = 0x42;
    constexpr int32_t MonoAssemblyNameBuild = 0x44;
    constexpr int32_t MonoAssemblyNameRevision = 0x46;
    constexpr int32_t MonoAssemblyNameArch = 0x48;

    // _MonoClass
    constexpr int32_t MonoClassSupertypes = 0x10;
    constexpr int32_t MonoClassIDepth = 0x18;
    constexpr int32_t MonoClassRank = 0x1A;
    constexpr int32_t MonoClassInstanceSize = 0x1C;
    constexpr int32_t MonoClassBits = 0x20;
    constexpr int32_t MonoClassMinAlign = 0x24;
    constexpr int32_t MonoClassParent = 0x30;
    constexpr int32_t MonoClassNestedIn = 0x38;
    constexpr int32_t MonoClassImage = 0x40;
    constexpr int32_t MonoClassName = 0x48;
    constexpr int32_t MonoClassNamespace = 0x50;
    constexpr int32_t MonoClassTypeToken = 0x58;
    constexpr int32_t MonoClassVTableSize = 0x5C;
    constexpr int32_t MonoClassInterfaceCount = 0x60;
    constexpr int32_t MonoClassMaxInterfaceId = 0x68;
    constexpr int32_t MonoClassInterfaces = 0x88;
    constexpr int32_t MonoClassFields = 0x98;
    constexpr int32_t MonoClassMethods = 0xA0;
    constexpr int32_t MonoClassRuntimeInfo = 0xD0;
    constexpr int32_t MonoClassVTable = 0xD8;

    // MonoClassDef
    constexpr int32_t MonoClassDefClass = 0x0;
    constexpr int32_t MonoClassDefFlags = 0xF0;
    constexpr int32_t MonoClassDefFieldMethodIdx = 0xF4;
    constexpr int32_t MonoClassDefFirstFieldIdx = 0xF8;
    constexpr int32_t MonoClassDefMethodCount = 0xFC;
    constexpr int32_t MonoClassDefFieldCount = 0x100;
    constexpr int32_t MonoClassDefNextCache = 0x108;

    // _MonoGenericClass
    constexpr int32_t MonoGenericClassContext = 0x8;
    constexpr int32_t MonoGenericClassCachedClass = 0x20;
    constexpr int32_t MonoGenericClassOwner = 0x28;

    // _MonoGenericContext
    constexpr int32_t MonoGenericContextClassInst = 0x0;
    constexpr int32_t MonoGenericContextMethodInst = 0x8;

    // _MonoClassField
    constexpr int32_t MonoClassFieldType = 0x0;
    constexpr int32_t MonoClassFieldName = 0x8;
    constexpr int32_t MonoClassFieldParent = 0x10;
    constexpr int32_t MonoClassFieldOffset = 0x18;

    // _MonoMethod
    constexpr int32_t MonoMethodFlags = 0x0;
    constexpr int32_t MonoMethodIFlags = 0x2;
    constexpr int32_t MonoMethodToken = 0x4;
    constexpr int32_t MonoMethodClass = 0x8;
    constexpr int32_t MonoMethodSignature = 0x10;
    constexpr int32_t MonoMethodName = 0x18;

    // _MonoObject
    constexpr int32_t MonoObjectVTable = 0x0;
    constexpr int32_t MonoObjectSynchronisation = 0x8;

    // _MonoType
    constexpr int32_t MonoTypeData = 0x0;
    constexpr int32_t MonoTypeAttributes = 0x8;
    constexpr int32_t MonoTypeModifiers = 0xC;

    // MonoClassRuntimeInfo
    constexpr int32_t MonoClassRuntimeInfoMaxDomain = 0x0;
    constexpr int32_t MonoClassRuntimeInfoDomainVTables = 0x8;

    // MonoVTable
    constexpr int32_t MonoVTableClass = 0x0;
    constexpr int32_t MonoVTableInitialized = 0x2D;
    constexpr int32_t MonoVTableFlags = 0x30;
    constexpr int32_t MonoVTableVTable = 0x40;

    // _MonoInternalHashTable
    constexpr int32_t MonoInternalHashTableKeyExtract = 0x8;
    constexpr int32_t MonoInternalHashTableNextValue = 0x10;
    constexpr int32_t MonoInternalHashTableSize = 0x18;
    constexpr int32_t MonoInternalHashTableNumEntries = 0x1C;
    constexpr int32_t MonoInternalHashTableTable = 0x20;

    // _MonoGHashTable
    constexpr int32_t MonoGHashTableKeys = 0x10;
    constexpr int32_t MonoGHashTableValues = 0x18;
    constexpr int32_t MonoGHashTableTableSize = 0x20;
    constexpr int32_t MonoGHashTableInUse = 0x24;
    constexpr int32_t MonoGHashTableKey = 0x40;

    // metadata
    constexpr uint64_t img_set_cache = 0x49A400;
} // namespace offsets

#endif