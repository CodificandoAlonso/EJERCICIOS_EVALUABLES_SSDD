/* src/common/claves_rpc.x */

struct RpcCoord {
    int x;
    int y;
};

struct entry {
    int      key;
    string   value1<256>;
    int      N_value2;
    double   V_value2<32>;
    RpcCoord value3;
};

struct GetRes {
    int      status;
    string   value1<256>;
    int      N_value2;
    double   V_value2<32>;
    RpcCoord value3;
};

program CLAVES_PROG {
    version CLAVES_VERS {
        int     SET_VALUE(entry)      = 1;
        GetRes  GET_VALUE(int)        = 2;
        int     DELETE_KEY(int)       = 3;
        int     MODIFY_VALUE(entry)   = 4;
        int     EXIST(int)            = 5;
        int     INIT_SERVICE(void)    = 6;
        int     DESTROY_SERVICE(void) = 7;
    } = 1;
} = 0x31234567;
