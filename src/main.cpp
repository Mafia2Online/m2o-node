#define ZPL_IMPLEMENTATION
#include "zpl.h"
#include "m2o_api.h"
#include "node_api.h"

#define m2o_assert ZPL_ASSERT
#define m2o_assert_msg ZPL_ASSERT_MSG

bool node_init();
void node_tick();
void node_stop();

#define TRY(exp) do { \
    napi_status status; \
    status = exp; \
    if (status != napi_ok) { \
        napi_throw_error(env, NULL, "N-API call failed"); \
    } \
} while(0)

#define REGISTER_METHOD(env, object, name, func) do { \
    napi_value fn; \
    TRY(napi_create_function(env, NULL, 0, func, NULL, &fn)); \
    TRY(napi_set_named_property(env, object, name, fn)); \
} while(0)

#define REGISTER_MEMBER(type, env, object, name, value) do { \
    napi_value val; \
    TRY(napi_create_##type (env, value, &val)); \
    TRY(napi_set_named_property(env, object, name, val)); \
} while (0)

m2o_api_vtable api = {0};

napi_value vehicle_create(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    int number = 0;
    napi_value argv[1];

    TRY(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    TRY(napi_get_value_int32(env, argv[0], &number));

    napi_value myNumber;
    number = number * 2;

    TRY(napi_create_int32(env, number, &myNumber));
    return myNumber;
}

napi_value vehicle_position_get(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    int vehicle_id = 0;
    napi_value argv[1];

    TRY(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    TRY(napi_get_value_int32(env, argv[0], &vehicle_id));

    // m2o_vec3 pos = api.vehicle_position_get(vehicle_id);
    m2o_vec3 pos = {0, 15, 242};

    napi_value result;
    TRY(napi_create_object(env, &result));

    REGISTER_MEMBER(double, env, result, "x", pos.x);
    REGISTER_MEMBER(double, env, result, "y", pos.y);
    REGISTER_MEMBER(double, env, result, "z", pos.z);

    return result;
}

napi_value m2on_init(napi_env env, napi_value exports) {
    REGISTER_METHOD(env, exports, "vehicle_create", vehicle_create);
    REGISTER_METHOD(env, exports, "vehicle_position_get", vehicle_position_get);
    // REGISTER_METHOD(env, exports, "vehicle_position_set", vehicle_position_set);
    // REGISTER_METHOD(env, exports, "vehicle_rotation_get", vehicle_rotation_get);
    // REGISTER_METHOD(env, exports, "vehicle_rotation_set", vehicle_rotation_set);
    return exports;
}

void plugin_init(const m2o_args *args, m2o_event_result *_) {
    zpl_printf("[info] m2o-node: initialized plugin\n");
    node_init();
}

void plugin_tick(const m2o_args *args, m2o_event_result *_) {
    node_tick();
}

void plugin_stop(const m2o_args *args, m2o_event_result *_) {
    node_stop();
}

void player_connect(const m2o_args *args, m2o_event_result *result) {
    zpl_printf("a new player %d has connected\n", (int)args->values[0].integer);
}

void player_disconnect(const m2o_args *args, m2o_event_result *result) {
    zpl_printf("a player %d has disconnected\n", (int)args->values[0].integer);
}

// dynmaic lib/testing execuatble switch
#if defined(TEST)

int main(int argc, char* argv[]) {
    node_init();

    while (true) {
        node_tick();
        zpl_sleep_ms(10);
    }

    node_stop();
    return 0;
}

#else

M2O_PLUGIN_MAIN(api, plugin) {
    plugin->name    = "m2o-node";
    plugin->author  = "inlife";
    plugin->version = M2O_VERSION_CREATE(1, 0, 0);

    plugin->callbacks.plugin_init = plugin_init;
    plugin->callbacks.plugin_tick = plugin_tick;
    plugin->callbacks.plugin_stop = plugin_stop;

    plugin->callbacks.player_connect    = player_connect;
    plugin->callbacks.player_disconnect = player_disconnect;
}

#endif

enum { NM_F_BUILTIN  = 1 << 0 };
NAPI_MODULE_X(m2o, m2on_init, NULL, NM_F_BUILTIN)
