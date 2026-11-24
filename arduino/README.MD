# Using header file as env for injection of secrets

## Create a secret_header.h file containing

```
#define SECRET_SSID "SuperSSID"
#define SECRET_PASS "hackme"
#define SECRET_URL "http://192.168.1.5:3000/api/readings"
```