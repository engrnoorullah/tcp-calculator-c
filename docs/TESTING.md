# Validation record

Validated on 27 October 2026 with GCC 13.3.0 on Linux.

- The maintained code builds with `-Wall -Wextra -Wpedantic -Werror`.
- `make test` passed in this environment.
- Tests run locally; they do not contact public services.
- Build products are excluded from version control.

Seven Python integration tests passed, including 24 concurrent client requests, byte-fragmented input, oversized and malformed requests, arithmetic boundaries and a connection ending mid-request.
