// copyright defined in LICENSE.txt

try {
    let wasm = get_wasm();
    let mod = new WebAssembly.Module(get_wasm());
    let inst;
    const env = {
        abort() {
            throw new Error('called abort');
        },
        eosio_assert(test, msg) {
            if (!test)
                throw new Error('assert failed');
        },
        get_blockchain_parameters_packed() {
            throw new Error('called get_blockchain_parameters_packed');
        },
        set_blockchain_parameters_packed() {
            throw new Error('called set_blockchain_parameters_packed');
        },
        exec_query(req_begin, req_end, cb_alloc_data, cb_alloc) {
            exec_query(inst.exports.memory.buffer, req_begin, req_end, size => {
                // cb_alloc may resize memory, causing inst.exports.memory.buffer to change
                let ptr = inst.exports.__indirect_function_table.get(cb_alloc)(cb_alloc_data, size);
                return [inst.exports.memory.buffer, ptr];
            });
        },
        print_range(begin, end) {
            print_wasm_str(inst.exports.memory.buffer, begin, end);
        },
    };
    inst = new WebAssembly.Instance(mod, { env });
    inst.exports.startup();
} catch (e) {
    print_js_str('Caught: ' + e + '\n');
}
