// copyright defined in LICENSE.txt

#include <eosiolib/asset.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/varint.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace eosio;
using namespace std;

extern "C" void* memcpy(void* __restrict dest, const void* __restrict src, size_t size) {
    auto d = reinterpret_cast<char*>(dest);
    auto s = reinterpret_cast<const char*>(src);
    while (size--)
        *d++ = *s++;
    return dest;
}

extern "C" void* memset(void* dest, int v, size_t size) {
    auto d = reinterpret_cast<char*>(dest);
    while (size--)
        *d++ = v;
    return dest;
}

extern "C" void print_range(const char* begin, const char* end);
extern "C" void prints(const char* cstr) { print_range(cstr, cstr + strlen(cstr)); }
extern "C" void prints_l(const char* cstr, uint32_t len) { print_range(cstr, cstr + len); }

extern "C" void printn(uint64_t n) {
    char buffer[13];
    auto end = name{n}.write_as_string(buffer, buffer + sizeof(buffer));
    print_range(buffer, end);
}

extern "C" void printui(uint64_t value) {
    char  s[21];
    char* ch = s;
    do {
        *ch++ = '0' + (value % 10);
        value /= 10;
    } while (value);
    std::reverse(s, ch);
    *ch = 0;
    print_range(s, ch);
}

extern "C" void printi(int64_t value) {
    if (value < 0) {
        prints("-");
        printui(-value);
    } else
        printui(value);
}

// todo: remove this
template <typename T>
struct serial_wrapper {
    T value{};
};

// todo: remove this
template <typename DataStream>
DataStream& operator<<(DataStream& ds, const serial_wrapper<checksum256>& obj) {
    eosio_assert(false, "oops");
    return ds;
}

// todo: remove this
template <typename DataStream>
DataStream& operator>>(DataStream& ds, serial_wrapper<checksum256>& obj) {
    ds.read(reinterpret_cast<char*>(obj.value.data()), obj.value.num_words() * sizeof(checksum256::word_t));
    return ds;
}

// todo: don't return static storage
// todo: replace with eosio functions when linker is improved
const char* asset_to_string(const asset& v) {
    static char result[1000];
    auto        pos = result;
    uint64_t    amount;
    if (v.amount < 0)
        amount = -v.amount;
    else
        amount = v.amount;
    uint8_t precision = v.symbol.precision();
    if (precision) {
        while (precision--) {
            *pos++ = '0' + amount % 10;
            amount /= 10;
        }
        *pos++ = '.';
    }
    do {
        *pos++ = '0' + amount % 10;
        amount /= 10;
    } while (amount);
    if (v.amount < 0)
        *pos++ = '-';
    *pos++ = ' ';

    auto sc = v.symbol.code().raw();
    while (sc > 0) {
        *pos++ = char(sc & 0xFF);
        sc >>= 8;
    }

    *pos++ = 0;
    return result;
}

namespace eosio {
template <typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, datastream<Stream>& dest) {
    unsigned_int size;
    ds >> size;
    dest = datastream<Stream>{ds.pos(), size};
    ds.skip(size);
    return ds;
}
} // namespace eosio

typedef void* cb_alloc_fn(void* cb_alloc_data, size_t size);

struct contract_row {
    uint32_t                block_index = 0;
    bool                    present     = false;
    name                    code;
    uint64_t                scope;
    name                    table;
    uint64_t                primary_key = 0;
    name                    payer;
    datastream<const char*> value{nullptr, 0};
};

struct action_trace {
    uint32_t                    block_index             = {};
    serial_wrapper<checksum256> transaction_id          = {};
    uint32_t                    action_index            = {};
    uint32_t                    parent_action_index     = {};
    datastream<const char*>     transaction_status      = {nullptr, 0}; // todo: enum
    eosio::name                 receipt_receiver        = {};
    serial_wrapper<checksum256> receipt_act_digest      = {};
    uint64_t                    receipt_global_sequence = {};
    uint64_t                    receipt_recv_sequence   = {};
    unsigned_int                receipt_code_sequence   = {};
    unsigned_int                receipt_abi_sequence    = {};
    eosio::name                 account                 = {};
    eosio::name                 name                    = {};
    datastream<const char*>     data                    = {nullptr, 0};
    bool                        context_free            = {};
    int64_t                     elapsed                 = {};
    datastream<const char*>     console                 = {nullptr, 0}; // todo: string
    datastream<const char*>     except                  = {nullptr, 0}; // todo: string

    EOSLIB_SERIALIZE(
        action_trace, (block_index)(transaction_id)(action_index)(parent_action_index)(transaction_status)(receipt_receiver)(
                          receipt_act_digest)(receipt_global_sequence)(receipt_recv_sequence)(receipt_code_sequence)(receipt_abi_sequence)(
                          account)(name)(data)(context_free)(elapsed)(console)(except))
};

struct code_table_pk_scope {
    name     code;
    name     table;
    uint64_t primary_key = 0;
    uint64_t scope;
};

struct code_table_scope_pk {
    name     code;
    name     table;
    uint64_t scope;
    uint64_t primary_key = 0;
};

struct scope_table_pk_code {
    uint64_t scope;
    name     table;
    uint64_t primary_key = 0;
    name     code;
};

struct receiver_name_account {
    eosio::name receipt_receiver = {};
    eosio::name name             = {};
    eosio::name account          = {};
};

struct query_contract_row_range_code_table_pk_scope {
    uint32_t            max_block_index = 0;
    code_table_pk_scope first;
    code_table_pk_scope last;
    uint32_t            max_results = 1;
};

struct query_contract_row_range_code_table_scope_pk {
    uint32_t            max_block_index = 0;
    code_table_scope_pk first;
    code_table_scope_pk last;
    uint32_t            max_results = 1;
};

struct query_contract_row_range_scope_table_pk_code {
    uint32_t            max_block_index = 0;
    scope_table_pk_code first;
    scope_table_pk_code last;
    uint32_t            max_results = 1;
};

struct query_action_trace_range_receiver_name_account {
    uint32_t              max_block_index = 0;
    receiver_name_account first           = {};
    receiver_name_account last            = {};
    uint32_t              max_results     = 1;
};

using query = std::variant<
    query_contract_row_range_code_table_pk_scope, query_contract_row_range_code_table_scope_pk,
    query_contract_row_range_scope_table_pk_code, query_action_trace_range_receiver_name_account>;

extern "C" void exec_query(void* req_begin, void* req_end, void* cb_alloc_data, cb_alloc_fn* cb_alloc);

template <typename Alloc_fn>
inline void exec_query(const query& req, Alloc_fn alloc_fn) {
    auto req_data = pack(req);
    exec_query(req_data.data(), req_data.data() + req_data.size(), &alloc_fn, [](void* cb_alloc_data, size_t size) -> void* { //
        return (*reinterpret_cast<Alloc_fn*>(cb_alloc_data))(size);
    });
}

inline std::vector<char> exec_query(const query& req) {
    std::vector<char> result;
    exec_query(req, [&result](size_t size) {
        result.resize(size);
        return result.data();
    });
    return result;
}

template <typename result, typename F>
bool for_each_query_result(const std::vector<char>& bytes, F f) {
    datastream<const char*> ds(bytes.data(), bytes.size());
    unsigned_int            size;
    ds >> size;
    for (uint32_t i = 0; i < size.value; ++i) {
        result r;
        ds >> r;
        if (!f(r))
            return false;
    }
    return true;
}

template <typename payload, typename F>
bool for_each_contract_row(const std::vector<char>& bytes, F f) {
    return for_each_query_result<contract_row>(bytes, [&](contract_row& row) {
        payload p;
        if (row.present && row.value.remaining()) {
            // todo: don't assert if serialization fails
            row.value >> p;
            if (!f(row, &p))
                return false;
        } else {
            if (!f(row, nullptr))
                return false;
        }
        return true;
    });
}

struct newaccount {
    eosio::name creator;
    eosio::name name;
    // authority owner;
    // authority active;
};

void balances_for_multiple_accounts(
    uint32_t max_block_index, name code, symbol_code sc, name first_account, name last_account, uint32_t max_results) {
    print("    balances_for_multiple_accounts\n");
    auto s = exec_query(query_contract_row_range_code_table_pk_scope{
        .max_block_index = max_block_index,
        .first =
            {
                .code        = code,
                .table       = "accounts"_n,
                .primary_key = sc.raw(),
                .scope       = first_account.value,
            },
        .last =
            {
                .code        = code,
                .table       = "accounts"_n,
                .primary_key = sc.raw(),
                .scope       = last_account.value,
            },
        .max_results = max_results,
    });
    for_each_contract_row<asset>(s, [&](contract_row& r, asset* a) {
        print("        ", r.block_index, " ", r.present, " ", r.code, " ", name{r.scope}, " ", r.payer);
        if (r.present && a)
            print(" ", asset_to_string(*a));
        print("\n");
        return true;
    });
    print("\n");
}

void proposals(uint32_t max_block_index, name first_account, name first_prop, name last_account, name last_prop, uint32_t max_results) {
    print("    proposals\n");
    auto s = exec_query(query_contract_row_range_code_table_scope_pk{
        .max_block_index = max_block_index,
        .first =
            {
                .code        = "eosio.msig"_n,
                .table       = "proposal"_n,
                .scope       = first_account.value,
                .primary_key = first_prop.value,
            },
        .last =
            {
                .code        = "eosio.msig"_n,
                .table       = "proposal"_n,
                .scope       = last_account.value,
                .primary_key = last_prop.value,
            },
        .max_results = max_results,
    });
    for_each_query_result<contract_row>(s, [&](contract_row& r) {
        print("        ", r.block_index, " ", r.present, " ", name{r.scope}, " ", name{r.primary_key});
        if (r.present)
            print(" ", r.value.remaining(), " bytes");
        print("\n");
        return true;
    });
    print("\n");
}

void balances_for_multiple_tokens(uint32_t max_block_index, name account, uint32_t max_results) {
    print("    balances_for_multiple_tokens\n");
    auto s = exec_query(query_contract_row_range_scope_table_pk_code{
        .max_block_index = max_block_index,
        .first =
            {
                .scope       = account.value,
                .table       = "accounts"_n,
                .primary_key = 0,
                .code        = name{0},
            },
        .last =
            {
                .scope       = account.value,
                .table       = "accounts"_n,
                .primary_key = ~uint64_t(0),
                .code        = name{~uint64_t(0)},
            },
        .max_results = max_results,
    });
    for_each_query_result<contract_row>(s, [&](contract_row& r) {
        if (!r.present || r.value.remaining() != 16)
            return true;
        asset a;
        r.value >> a;
        if (!a.is_valid() || a.symbol.code().raw() != r.primary_key)
            return true;
        print("        ", name{r.scope}, " ", r.code, " ", asset_to_string(a), "\n");
        return true;
    });
    print("\n");
}

void creators(uint32_t max_block_index, uint32_t max_results) {
    print("    creators\n");
    auto s = exec_query(query_action_trace_range_receiver_name_account{
        .max_block_index = max_block_index,
        .first =
            {
                .receipt_receiver = "eosio"_n,
                .name             = "newaccount"_n,
                .account          = "eosio"_n,
            },
        .last =
            {
                .receipt_receiver = "eosio"_n,
                .name             = "newaccount"_n,
                .account          = "eosio"_n,
            },
        .max_results = max_results,
    });
    for_each_query_result<action_trace>(s, [&](action_trace& t) {
        char status[20];
        auto size = min(t.transaction_status.remaining(), sizeof(status) - 1);
        t.transaction_status.read(status, size);
        status[size] = 0;
        if (strcmp(status, "executed"))
            return true;
        newaccount na;
        t.data >> na;
        print("        ", na.creator, ", ", na.name, "\n");
        return true;
    });
    print("\n");
}

extern "C" void startup() {
    print("\nstart wasm\n");
    creators(30000000, 20);
    balances_for_multiple_accounts(3000000, "eosio.token"_n, symbol_code{"EOS"}, "eosio"_n, "eosio.zzzzzz"_n, 100);
    proposals(30000000, "h"_n, name{0}, "hzzzzzzzzzzz"_n, name{~uint64_t(0)}, 20);
    balances_for_multiple_tokens(30000000, "eosio"_n, 100);
    print("end wasm\n\n");
}
