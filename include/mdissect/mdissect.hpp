#ifndef MEMORY_MONO_HPP_
#define MEMORY_MONO_HPP_

#include <functional>
#include <iostream>
#include <vector>

#define GET_CURRENT_CLASS(ClassName, Assembly, Token)                    \
    static mdissect::mono_class get_mono_klass() {                       \
        static const auto klass = mdissect::find_class(Assembly, Token); \
        return klass;                                                    \
    }

// TODO: Generics
#define GET_CURRENT_GENERIC_CLASS(ClassName, Assembly, Token, ...)       \
    static mdissect::mono_class get_mono_klass() {                       \
        static const auto klass = mdissect::find_class(Assembly, Token); \
        return klass;                                                    \
    }

#define STATIC_FIELD(T, NAME, ClassName, Token)

#define HASH_TABLE_SIZE 1103

// TODO: Merge mdissect over to using memory blocks to increase performance a little more
// https://github.com/migueldeicaza/mono-wasm-mono/blob/8eb6ac3a88b9e6ca5127dce5a3d3c075bf1586a4/mono/metadata/class-internals.h#L253
namespace mdissect {
    using fn_read_memory = bool (*)(uint64_t address, void* buffer, size_t size);
    using fn_write_memory = bool (*)(uint64_t address, const void* buffer, size_t size);

    extern fn_read_memory read_memory;
    extern fn_write_memory write_memory;

    struct mono_class;

    struct mono_vtable {
        mono_vtable() = default;
        explicit mono_vtable(uint64_t address) : address(address) {};

        uint64_t address;

        [[nodiscard]] bool initialized() const;
        [[nodiscard]] mono_class mono_class() const;

        [[nodiscard]] uint64_t static_field_data() const;
    };

    struct mono_class;

    struct mono_image {
        mono_image() = default;
        explicit mono_image(uint64_t address) : address(address) {};

        uint64_t address;

        [[nodiscard]] std::string filename() const;
        [[nodiscard]] std::vector<mono_class> types() const;

        using fn_match_callback = std::function<bool(mono_class)>;
        [[nodiscard]] mono_class get_type(const fn_match_callback& callback) const;
    };

    struct mono_assembly {
        mono_assembly() = default;
        explicit mono_assembly(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] std::string name() const;
        [[nodiscard]] mono_image image() const;
    };

    struct mono_domain {
        mono_domain() = default;
        explicit mono_domain(uint64_t address) : address(address) {}

        uint64_t address;

        bool operator==(const mono_domain other) const {
            return address == other.address;
        }

        bool operator!=(const mono_domain other) const {
            return !(*this == other);
        }

        [[nodiscard]] int32_t domain_id() const;
        [[nodiscard]] std::vector<mono_assembly> domain_assemblies() const;
    };

    struct mono_method {
        mono_method() = default;
        explicit mono_method(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] uint32_t token() const;
        [[nodiscard]] mono_class mono_class() const;
        [[nodiscard]] std::string name() const;
    };

    struct mono_type {
        mono_type() = default;
        explicit mono_type(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] mono_class mono_class() const;
        [[nodiscard]] uint32_t attributes() const;
        [[nodiscard]] bool is_static() const;
        [[nodiscard]] bool is_literal() const;
    };

    struct mono_class_field {
        mono_class_field() = default;
        explicit mono_class_field(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] mono_type type() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] mono_class parent() const;
        [[nodiscard]] int32_t offset() const;
    };

    struct mono_class {
        mono_class() = default;
        explicit mono_class(uint64_t address) : address(address) {}

        bool operator==(const mono_class& other) const {
            return address == other.address;
        }

        uint64_t address;

        [[nodiscard]] mono_class parent() const;
        [[nodiscard]] mono_class nesting_type() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string name_space() const;
        [[nodiscard]] uint32_t type_token() const;
        [[nodiscard]] int32_t vtable_size() const;
        [[nodiscard]] std::vector<mono_class_field> fields() const;
        [[nodiscard]] std::vector<mono_method> methods() const;
        [[nodiscard]] mono_vtable vtable(mono_domain domain) const;

        [[nodiscard]] uint32_t class_kind() const;
        [[nodiscard]] uint32_t method_count() const;

        [[nodiscard]] bool is_blittable() const;
        [[nodiscard]] bool is_delegate() const;
        [[nodiscard]] bool is_enum() const;
        [[nodiscard]] bool is_generic() const;
        [[nodiscard]] bool is_inflated() const;
        [[nodiscard]] bool is_valuetype() const;

        using fn_match_field_callback = std::function<bool(mono_class_field)>;
        [[nodiscard]] mono_class_field get_field(const fn_match_field_callback& callback) const;

        [[nodiscard]] mono_class_field get_field(std::string_view field_name) const;
        [[nodiscard]] mono_class_field get_field(uint32_t token) const;

        using fn_match_method_callback = std::function<bool(mono_method)>;
        [[nodiscard]] mono_method get_method(const fn_match_method_callback& callback) const;

        // TODO: Setup signatures to find by args
        // TODO: Param count search option
        [[nodiscard]] mono_method get_method(std::string_view method_name) const;
        [[nodiscard]] mono_method get_method(uint32_t token) const;
    };

    struct mono_generic_context {
        mono_generic_context() = default;
        explicit mono_generic_context(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] uint64_t class_inst() const;
        [[nodiscard]] uint64_t method_inst() const;
    };

    struct mono_generic_class {
        mono_generic_class() = default;
        explicit mono_generic_class(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] mono_generic_context context() const;
        [[nodiscard]] mono_class cached_class() const;
    };

    struct mono_object {
        mono_object() = default;
        explicit mono_object(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] mono_vtable vtable() const;
    };

    struct mono {
        mono() = default;
        explicit mono(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] mono_domain root_domain(uint64_t offset) const;
    };

    struct mono_internal_hash_table {
        mono_internal_hash_table() = default;
        explicit mono_internal_hash_table(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] int32_t size() const;
        [[nodiscard]] uint64_t table() const;
    };

    struct mono_image_set {
        mono_image_set() = default;
        explicit mono_image_set(uint64_t address) : address(address) {}

        uint64_t address;

        [[nodiscard]] uint64_t hash_table() const;
    };

    bool as_internal(const mono_class& klass, const mono_class& parent);

    template <typename T>
    bool as(const mono_class& klass) {
        return as_internal(klass, T::get_mono_klass());
    }

    bool attach(uint64_t runtime);

    mono_domain get_root_domain();
    mono_image_set get_image_set_cache_entry(int32_t index);

    mdissect::mono_image get_assembly_image(std::string_view assembly_name);

    uint64_t get_static_field_data(mdissect::mono_class mono_class);
    uint64_t get_static_field_address(mdissect::mono_class_field field, mdissect::mono_class mono_class);

    mono_class find_class(std::string_view namespace_name, std::string_view class_name);
    mono_class find_class(std::string_view assembly_name, uint32_t token);
    // TODO: Generics class instances

    mono_method find_method(std::string_view namespace_name, std::string_view class_name, std::string_view method_name);
    mono_method find_method(std::string_view assembly_name, uint32_t token);
} // namespace mdissect

#endif