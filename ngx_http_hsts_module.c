
/*
* Copyright (C) Yanbing Shi
*/


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static void *ngx_http_hsts_create_srv_conf(ngx_conf_t *cf);
static char *ngx_http_hsts_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t ngx_http_hsts_filter_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_hsts_header_filter(ngx_http_request_t *r);

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;


// Module configuration struct
typedef struct {
    ngx_flag_t             enable;
    ngx_uint_t             maxAge;
    ngx_flag_t             includeSubdomains;
    ngx_flag_t             preload;
} ngx_http_hsts_srv_conf_t;


// Module directives
static ngx_command_t  ngx_http_hsts_commands[] = {

    { ngx_string("hsts"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_SRV_CONF_OFFSET,
    offsetof(ngx_http_hsts_srv_conf_t, enable),
    NULL },

    { ngx_string("hsts_max_age"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_SRV_CONF_OFFSET,
    offsetof(ngx_http_hsts_srv_conf_t, maxAge),
    NULL },

    { ngx_string("hsts_includesubdomains"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_SRV_CONF_OFFSET,
    offsetof(ngx_http_hsts_srv_conf_t, includeSubdomains),
    NULL },

    { ngx_string("hsts_preload"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_SRV_CONF_OFFSET,
    offsetof(ngx_http_hsts_srv_conf_t, preload),
    NULL },

    ngx_null_command
};


// Module context
static ngx_http_module_t  ngx_http_hsts_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_hsts_filter_init,              /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    ngx_http_hsts_create_srv_conf,         /* create server configuration */
    ngx_http_hsts_merge_srv_conf,          /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


// Module definition
ngx_module_t  ngx_http_hsts_module = {
    NGX_MODULE_V1,
    &ngx_http_hsts_module_ctx,             /* module context */
    ngx_http_hsts_commands,                /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


// Allocate module configuration
static void *
ngx_http_hsts_create_srv_conf(ngx_conf_t *cf)
{
    ngx_http_hsts_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hsts_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;
    conf->maxAge = NGX_CONF_UNSET_UINT;
    conf->includeSubdomains = NGX_CONF_UNSET;
    conf->preload = NGX_CONF_UNSET;

    return conf;
}


// Merge module configuration
static char *
ngx_http_hsts_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hsts_srv_conf_t *prev = parent;
    ngx_http_hsts_srv_conf_t *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);
    ngx_conf_merge_uint_value(conf->maxAge, prev->maxAge, 0);
    ngx_conf_merge_value(conf->includeSubdomains, prev->includeSubdomains, 0);
    ngx_conf_merge_value(conf->preload, prev->preload, 0);

    return NGX_CONF_OK;
}


// Filter registration
static ngx_int_t
ngx_http_hsts_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_hsts_header_filter;

    return NGX_OK;
}


// Actual header filter processing
static ngx_int_t
ngx_http_hsts_header_filter(ngx_http_request_t *r)
{
    ngx_http_hsts_srv_conf_t *conf;
    ngx_table_elt_t *h;
    u_char *hv;
    u_char *p;


    // Skip HTTP clear text requests
    if (r->connection->ssl == NULL) {
        return ngx_http_next_header_filter(r);
    }

    // Retrieve module configuration
    conf = ngx_http_get_module_srv_conf(r, ngx_http_hsts_module);

    // Skip if module is not enabled
    if (conf->enable == 0) {
        return ngx_http_next_header_filter(r);
    }

    // Allocate HSTS header value buffer
    hv = ngx_pnalloc(r->pool, 64);
    if (hv == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // Generate HSTS header value
    p = ngx_snprintf(
        hv,
        64,
        "max-age=%ui%s%s",
        conf->maxAge,
        conf->includeSubdomains ? "; includeSubDomains" : "",
        conf->preload ? "; preload" : ""
        );

    // Add a response header entry to the header list
    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    // Set the HSTS response header
    h->hash = 1;
    ngx_str_set(&h->key, "Strict-Transport-Security");
    h->value.len = p - hv;
    h->value.data = (u_char*)hv;

    return ngx_http_next_header_filter(r);
}
