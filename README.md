# Nginx HSTS Module

The ngx_http_hsts_module provides HTTP Strict Transport Security (HSTS) support for Nginx.

Based on the HSTS specification, [RFC 6797](https://tools.ietf.org/html/rfc6797), the module adds the "Strict-Transport-Security" HTTP response header when Nginx replies a request through a secure connection (SSL/TLS). The header will not be added when Nginx replies an HTTP clear-text request.

### Directives

```
Syntax:	 hsts on | off;
Default: hsts off;
Context: http, server
```
Enables or disables HSTS.

```
Syntax:	 hsts_max_age delta-seconds;
Default: hsts_max_age 0;
Context: http, server
```
Specifies HSTS max-age directive.

```
Syntax:	 hsts_includesubdomains on | off;
Default: hsts_includesubdomains off;
Context: http, server
```
Enables or disables HSTS includeSubDomains directive.

```
Syntax:	 hsts_preload on | off;
Default: hsts_preload off;
Context: http, server
```
Enables or disables HSTS preload directive.

### Installing

Unpack the ngx_http_hsts_module:
```
$ unzip ngx_http_hsts_module-master.zip
```

Unpack Nginx sources:
```
$ tar zxvf nginx-1.x.x.tar.gz
$ cd ./nginx-1.x.x
```

Run Nginx configuration script with an additional "--add-module" switch to install third-party modules:
```
$ ./configure --add-module=/path-to-nginx-hsts-module/nginx-http-hsts-master [other configure options]
```

Compile and install Nginx:
```
$ make
$ sudo make install
```

Configure Nginx using the configuration directives of ngx_http_hsts_module under the context of http or server.
