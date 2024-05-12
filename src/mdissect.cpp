#include <mdissect/mdissect.hpp>
#include <mdissect/offsets.hpp>

#include <cassert>

namespace mdissect {
    fn_read_memory read_memory = nullptr;
    fn_write_memory write_memory = nullptr;

    __forceinline bool read(uint64_t address, void* buffer, size_t size) {
        return read_memory(address, buffer, size);
    }

    __forceinline bool write(uint64_t address, const void* buffer, size_t size) {
        return write_memory(address, buffer, size);
    }

    template <typename T>
    __forceinline static T read(uint64_t address) {
        T result;
        if (!read_memory(address, &result, sizeof(T)))
            return {};

        return result;
    }

    template <typename T>
    __forceinline static bool write(uint64_t address, const T& value) {
        return write_memory(address, &value, sizeof(T));
    }

    // mono_vtable
    bool mono_vtable::initialized() const {
        return read<uint8_t>(address + offsets::MonoVTableInitialized) != 0;
    }

    mono_class mono_vtable::mono_class() const {
        return mdissect::mono_class(read<uint64_t>(address + offsets::MonoVTableClass));
    }

    uint64_t mono_vtable::static_field_data() const {
        if ((read<uint8_t>(address + offsets::MonoVTableFlags) & 0x4) != 0) {
            const auto parent_class = mono_class();
            return read<uint64_t>(address + offsets::MonoVTableVTable + parent_class.vtable_size() * 0x8);
        }

        return 0;
    }

    // mono_image
    std::string mono_image::filename() const {
        char name[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoImageName), name, sizeof(name) - 1))
            return {};

        return name;
    }

    std::vector<mono_class> mono_image::types() const {
        std::vector<mono_class> result;

        const auto cache = mono_internal_hash_table(address + offsets::MonoImageClassCache);
        const auto cache_size = cache.size();
        const auto cache_table = cache.table();

        for (int32_t i = 0; i < cache_size; ++i) {
            for (auto it = read<uint64_t>(cache_table + i * 0x8); it != 0; it = read<uint64_t>(it + offsets::MonoClassDefNextCache)) {
                result.emplace_back(it);
            }
        }

        return result;
    }

    mono_class mono_image::get_type(const fn_match_callback& callback) const {
        const auto cache = mono_internal_hash_table(address + offsets::MonoImageClassCache);
        const auto cache_size = cache.size();
        const auto cache_table = cache.table();

        for (int32_t j = 0; j < cache_size; ++j) {
            for (auto it = read<uint64_t>(cache_table + j * 0x8); it != 0; it = read<uint64_t>(it + offsets::MonoClassDefNextCache)) {
                const auto mono_class = mdissect::mono_class(it);

                if (callback(mono_class))
                    return mono_class;
            }
        }

        return mono_class(0);
    }

    // mono_assembly
    std::string mono_assembly::name() const {
        char name[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoAssemblyName), name, sizeof(name) - 1))
            return {};

        return name;
    }

    mono_image mono_assembly::image() const {
        return mono_image(read<uint64_t>(address + offsets::MonoAssemblyImage));
    }

    // mono_domain
    int32_t mono_domain::domain_id() const {
        return read<int32_t>(address + offsets::MonoDomainDomainId);
    }

    std::vector<mono_assembly> mono_domain::domain_assemblies() const {
        std::vector<mono_assembly> result;

        const auto assemblies = read<uint64_t>(address + offsets::MonoDomainDomainAssemblies);
        for (uint64_t it = assemblies; it != 0; it = read<uint64_t>(it + 0x8)) {
            const auto assembly = read<uint64_t>(it);
            if (assembly == 0)
                continue;

            result.emplace_back(assembly);
        }

        return result;
    }

    // mono_method
    uint32_t mono_method::token() const {
        return read<uint32_t>(address + offsets::MonoMethodToken);
    }

    mono_class mono_method::mono_class() const {
        return mdissect::mono_class(read<uint64_t>(address + offsets::MonoMethodClass));
    }

    std::string mono_method::name() const {
        char name[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoMethodName), name, sizeof(name) - 1))
            return {};

        return name;
    }

    // mono_type
    mono_class mono_type::mono_class() const {
        return mdissect::mono_class(read<uint64_t>(address + offsets::MonoTypeData));
    }

    uint32_t mono_type::attributes() const {
        return read<uint32_t>(address + offsets::MonoTypeAttributes);
    }

    bool mono_type::is_static() const {
        return (attributes() & 0x10) != 0;
    }

    bool mono_type::is_literal() const {
        return (attributes() & 0x40) != 0;
    }

    // mono_field
    mono_type mono_class_field::type() const {
        return mono_type(read<uint64_t>(address + offsets::MonoClassFieldType));
    }

    std::string mono_class_field::name() const {
        char name[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoClassFieldName), name, sizeof(name) - 1))
            return {};

        return name;
    }

    mono_class mono_class_field::parent() const {
        return mono_class(read<uint64_t>(address + offsets::MonoClassFieldParent));
    }

    int32_t mono_class_field::offset() const {
        return read<int32_t>(address + offsets::MonoClassFieldOffset);
    }

    uint32_t mono_class_field::token() const {
        auto parent = this->parent();
        if (!parent.address)
            return 0;

        uint32_t first_field_index = 0;
        int32_t field_count = 0;

        int32_t index = 0;

        while (true) {
            if (parent.fields().size() == 0)
                return 0;

            first_field_index = parent.first_field_idx();
            field_count = parent.field_count();
            index = 0;

            if (field_count > 0)
                break;

        next_parent:
            parent = parent.parent();
            if (parent.address == 0)
                return 0;
        }

        while (parent.fields().at(index).address != this->address) {
            if (++index >= field_count)
                goto next_parent;
        }

        const int32_t v6 = index + 1;
        const auto image = parent.image();

        const uint32_t v8 = first_field_index + v6;
        return v8 | 0x4000000;
    }

    // mono_class
    mono_class mono_class::parent() const {
        return mono_class(read<uint64_t>(address + offsets::MonoClassParent));
    }

    mono_class mono_class::nesting_type() const {
        return mono_class(read<uint64_t>(address + offsets::MonoClassNestedIn));
    }

    mono_image mono_class::image() const {
        return mono_image(read<uint64_t>(address + offsets::MonoClassImage));
    }

    std::string mono_class::name() const {
        char name[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoClassName), name, sizeof(name) - 1))
            return {};

        return name;
    }

    std::string mono_class::name_space() const {
        char nspace[256] {};
        if (!read(read<uint64_t>(address + offsets::MonoClassNamespace), nspace, sizeof(nspace) - 1))
            return {};

        return nspace;
    }

    uint32_t mono_class::type_token() const {
        return read<uint32_t>(address + offsets::MonoClassTypeToken);
    }

    int32_t mono_class::vtable_size() const {
        return read<int32_t>(address + offsets::MonoClassVTableSize);
    }

    int32_t mono_class::field_count() const {
        /*unsigned int v1; // eax

        v1 = *((_WORD *)&klass->klass + 21) & 7;
        if ( !v1 )
            goto LABEL_5;
        while ( v1 > 2 )
        {
            if ( v1 != 3 )
            {
                if ( v1 - 4 <= 2 )
                    return 0i64;
                LABEL_5:
                      monoeg_assertion_message(
                        "* Assertion: should not be reached at %s:%d\n",
                        "..\\mono\\metadata\\class-accessors.c",
                        211i64);
            }
            klass = **(MonoClassDef ***)&klass->flags;
            v1 = *((_WORD *)&klass->klass + 21) & 7;
            if ( !v1 )
                goto LABEL_5;
        }
        return klass->field_count;*/

        return read<int32_t>(address + offsets::MonoClassDefFieldCount);
    }

    uint32_t mono_class::first_field_idx() const {
        // TODO: Should check class flags
        return read<uint32_t>(address + offsets::MonoClassDefFirstFieldIdx);
    }

    std::vector<mono_class_field> mono_class::fields() const {
        const auto fields = read<uint64_t>(address + offsets::MonoClassFields);
        if (fields == 0)
            return {};

        const auto field_count = read<int32_t>(address + offsets::MonoClassDefFieldCount);

        std::vector<mono_class_field> result;
        result.reserve(field_count);

        for (int32_t i = 0; i < field_count; ++i)
            result.emplace_back(fields + i * 0x20);

        return result;
    }

    std::vector<mono_method> mono_class::methods() const {
        const auto count = method_count();
        const auto methods = read<uint64_t>(address + offsets::MonoClassMethods);
        if (methods == 0)
            return {};

        std::vector<mono_method> result;
        result.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
            result.emplace_back(read<uint64_t>(methods + i * 0x8));

        return result;
    }

    mono_vtable mono_class::vtable(mono_domain domain) const {
        const auto runtime_info = read<uint64_t>(address + offsets::MonoClassRuntimeInfo);
        if (runtime_info == 0)
            return mono_vtable(0);

        const int32_t domain_id = domain.domain_id();
        const auto max_domain = read<uint16_t>(runtime_info + offsets::MonoClassRuntimeInfoMaxDomain);
        if (max_domain < domain_id)
            return mono_vtable(0);

        return mono_vtable(read<uint64_t>(runtime_info + offsets::MonoClassRuntimeInfoDomainVTables + domain_id * 0x8));
    }

    uint32_t mono_class::class_kind() const {
        return ((read<uint32_t>(address + 0x15)) & 7) - 1;
    }

    bool mono_class::is_blittable() const {
        return (read<uint32_t>(address + 0x8) >> 4) & 1;
    }

    bool mono_class::is_delegate() const {
        return (read<uint32_t>(address + 10) >> 8) * 1;
    }

    bool mono_class::is_enum() const {
        return (read<uint32_t>(address + 8) >> 3) & 1;
    }

    bool mono_class::is_generic() const {
        return (read<uint32_t>(address + 10) & 0x70000) == 0x20000;
    }

    bool mono_class::is_inflated() const {
        return (read<uint32_t>(address + 10) & 0x70000) == 196608;
    }

    bool mono_class::is_valuetype() const {
        return (read<uint32_t>(address + 8) >> 2) & 1;
    }

    // mono_class_get_method_count
    uint32_t mono_class::method_count() const {
        auto kind = class_kind();
        mono_class element_class = *this;

        while (true) {
            switch (kind) {
                case 0:
                case 1:
                    return read<uint32_t>(element_class.address + offsets::MonoClassDefMethodCount);
                case 2:
                    element_class = mono_class(read<uint64_t>(read<uint64_t>(element_class.address + offsets::MonoClassDefFlags)));
                    kind = element_class.class_kind();
                    if (kind > 5)
                        return 0;
                    continue;
                case 3:
                case 5:
                    return 0;
                case 4:
                    return read<uint32_t>(element_class.address + offsets::MonoClassDefFlags);
                default:
                    return 0;
            }
        }
    }

    mono_class_field mono_class::get_field(const fn_match_field_callback& callback) const {
        const auto fields = read<uint64_t>(address + offsets::MonoClassFields);
        const auto field_count = read<int32_t>(address + offsets::MonoClassDefFieldCount);
        for (int32_t i = 0; i < field_count; ++i) {
            const auto mono_field = mdissect::mono_class_field(fields + i * 0x20);
            if (callback(mono_field))
                return mono_field;
        }

        return mono_class_field(0);
    }

    mono_class_field mono_class::get_field(std::string_view field_name) const {
        return get_field([&field_name](const mono_class_field& field) {
            return field.name() == field_name;
        });
    }

    mono_class_field mono_class::get_field(uint32_t token) const {
        // TODO: Rebuild mono_class_get_field_token
        return mono_class_field(0);
    }


    mono_method mono_class::get_method(const fn_match_method_callback& callback) const {
        const auto method_count = read<uint32_t>(address + offsets::MonoClassDefMethodCount);
        const auto methods = read<uint64_t>(address + offsets::MonoClassMethods);
        if (methods == 0)
            return mono_method(0);

        for (uint32_t i = 0; i < method_count; ++i) {
            const auto mono_method = mdissect::mono_method(read<uint64_t>(methods + i * 0x8));
            if (callback(mono_method))
                return mono_method;
        }

        return mono_method(0);
    }

    mono_method mono_class::get_method(std::string_view method_name) const {
        return get_method([&method_name](const mono_method& method) {
            return method.name() == method_name;
        });
    }

    mono_method mono_class::get_method(uint32_t token) const {
        return get_method([token](const mono_method& method) {
            return method.token() == token;
        });
    }

    // mono_generic_context
    uint64_t mono_generic_context::class_inst() const {
        return read<uint64_t>(address + offsets::MonoGenericContextClassInst);
    }

    uint64_t mono_generic_context::method_inst() const {
        return read<uint64_t>(address + offsets::MonoGenericContextMethodInst);
    }

    // mono_generic_class
    mono_generic_context mono_generic_class::context() const {
        return mono_generic_context(read<uint64_t>(address + offsets::MonoGenericClassContext));
    }

    mono_class mono_generic_class::cached_class() const {
        return mono_class(read<uint64_t>(address + offsets::MonoGenericClassCachedClass));
    }

    // mono_object
    mono_vtable mono_object::vtable() const {
        return mono_vtable(read<uint64_t>(address + offsets::MonoObjectVTable));
    }

    // mono
    mono_domain mono::root_domain(uint64_t offset) const {
        return mono_domain(read<uint64_t>(address + offset));
    }

    // mono_hash_table
    int32_t mono_internal_hash_table::size() const {
        return read<int32_t>(address + offsets::MonoInternalHashTableSize);
    }

    uint64_t mono_internal_hash_table::table() const {
        return read<uint64_t>(address + offsets::MonoInternalHashTableTable);
    }

    // mono_image_set
    uint64_t mono_image_set::hash_table() const {
        return read<uint64_t>(address + 0x28);
    }

    // helpers
    bool as_internal(const mono_class& klass, const mono_class& parent) { // NOLINT(*-no-recursion)
        // Sometimes objects so when getting the vtable to get the class it's incorrect
        // L33T fix
        if (klass.address <= 0x40000)
            return false;

        if (klass == parent)
            return true;

        const auto parent_klass = klass.parent();
        if (parent_klass.address == 0)
            return false;

        return as_internal(parent_klass, parent);
    }

    // default domain and image for helper functions
    mdissect::mono _mono;
    mdissect::mono_domain _root_domain;
    mdissect::mono_image _assembly_image;

    bool attach(const uint64_t runtime) {
        _mono = mdissect::mono(runtime);
        if (_mono.address == 0)
            return false;

        _root_domain = _mono.root_domain(0x499C78);
        if (_root_domain.address == 0)
            return false;

        _assembly_image = get_assembly_image("Assembly-CSharp");
        if (_assembly_image.address == 0)
            return false;

        return true;
    }

    mono_domain get_root_domain() {
        return _root_domain;
    }

    mono_image_set get_image_set_cache_entry(int32_t index) {
        return mono_image_set(read<uint64_t>(_mono.address + offsets::img_set_cache + index * 0x8));
    }

    mdissect::mono_image get_assembly_image(std::string_view assembly_name) {
        for (const auto assembly: _root_domain.domain_assemblies()) {
            const auto name = assembly.name();
            if (name != assembly_name)
                continue;

            return assembly.image();
        }

        return {};
    }

    uint64_t get_static_field_data(mono_class mono_class) {
        const auto vtable = mono_class.vtable(_root_domain);
        if (vtable.address == 0)
            return 0;

        return vtable.static_field_data();
    }

    uint64_t get_static_field_address(mono_class_field field, mono_class mono_class) {
        if (!field.address || !mono_class.address)
            return 0;

        const uint64_t static_field_data = get_static_field_data(mono_class);
        if (static_field_data == 0)
            return 0;

        return static_field_data + field.offset();
    }

    mono_class find_class(std::string_view namespace_name, std::string_view class_name) {
        for (const auto& assembly: mdissect::get_root_domain().domain_assemblies()) {
            const auto image = assembly.image();

            for (const auto& type: image.types()) {
                if (type.name_space() == namespace_name && type.name() == class_name)
                    return type;
            }
        }

        return {};
    }

    mono_class find_class(std::string_view assembly_name, uint32_t token) {
        auto assembly = get_assembly_image(assembly_name);
        if (assembly.address == 0)
            return {};

        for (const auto type: assembly.types()) {
            if (type.type_token() == token)
                return type.vtable(_root_domain).mono_class();
        }

        return {};
    }

    mono_method find_method(std::string_view namespace_name, std::string_view class_name, std::string_view method_name) {
        const auto klass = find_class(namespace_name, class_name);
        if (klass.address == 0)
            return mono_method(0);

        return klass.get_method(method_name);
    }

    mono_method find_method(std::string_view assembly_name, uint32_t token) {
        const auto image = get_assembly_image(assembly_name);
        if (image.address == 0)
            return mono_method(0);

        for (const auto& type: image.types()) {
            for (const auto& method: type.methods()) {
                if (method.token() == token)
                    return method;
            }
        }

        return mono_method(0);
    }
    mono_class find_class_containing_method(std::string_view method_name) {
        for (const auto assembly : _root_domain.domain_assemblies()) {
            if (assembly.address == 0)
                continue;

            const auto image = assembly.image();
            if (image.address == 0)
                continue;

            for (const auto type : image.types()) {
                if (type.address == 0)
                    continue;

                for (const auto method : type.methods()) {
                    if (method.name() == method_name)
                        return type;
                }
            }
        }

        return mono_class(0);
    }

    mono_class find_class_containing_field(std::string_view field_name) {
        for (const auto assembly : _root_domain.domain_assemblies()) {
            if (assembly.address == 0)
                continue;

            const auto image = assembly.image();
            if (image.address == 0)
                continue;

            for (const auto type : image.types()) {
                if (type.address == 0)
                    continue;

                for (const auto field : type.fields()) {
                    if (field.name() == field_name)
                        return type;
                }
            }
        }

        return mono_class(0);
    }
} // namespace mdissect
