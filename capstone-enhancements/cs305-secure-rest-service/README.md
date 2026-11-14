# Secure REST Service (CS-499 Milestone Two — Software Design & Engineering)

Enhances and merges two CS-305 course projects into one hardened Spring Boot
service:

- `coursework/CS-305_Software-Security/CS 305 Project One Code Base` — a
  skeleton CRUD REST controller with no security, no validation, and a
  dead, never-called `DocData.read_document()` method that opened an
  unauthenticated JDBC connection (`root`/`root`) it never used.
- `coursework/CS-305_Software-Security/CS 305 Project Two Code Base` — an
  independent SSL/TLS server with blank keystore placeholders (`????`) and
  no working endpoint.

The planned enhancements below follow the Module 1 Enhancement Plan and the
specific commitments made on camera in the Module 2 code-review video
(`Downloads\CS-499\CS499_Milestone1_Video_Outline.md`, Category 2 section).

Neither project ran behind the other. This service merges them so every CRUD
endpoint runs over one real TLS configuration, and adds everything the
original pair lacked: authentication, role-based authorization, input
validation, centralized error handling, and an enforced dependency-risk gate.

## What changed and why

| Area | Original | Enhanced |
|---|---|---|
| TLS | Project Two's keystore config was placeholder (`????`) and never connected to the CRUD service | Real PKCS12 keystore wired into `application.properties`; every `/api/documents/**` request runs over HTTPS (port 8443) |
| Auth | None — `CRUDController` had no security configuration at all | Spring Security, HTTP Basic, BCrypt-hashed credentials, two roles (`USER`, `ADMIN`) modeled on the course's GameAuth pattern but with real per-account secrets instead of one hardcoded `"password"` for every account |
| Authorization | None | `GET /api/documents` requires `USER` or `ADMIN`; `POST /api/documents` (create) requires `ADMIN` |
| Input validation | `@RequestParam` with no constraints | Bean Validation (`@NotBlank`, `@Size`) on a dedicated request DTO, enforced before the service layer ever runs |
| Error handling | None — an exception would hit Spring Boot's default error page (class name + message) | `@RestControllerAdvice` returns a fixed `{timestamp, status, error, message}` shape; the real exception is logged server-side only, never returned to the caller |
| Dependency risk | `dependency-check-maven` was declared but had no `failBuildOnCVSS`, so the default threshold (11, a score that doesn't exist) meant the build could never fail | `failBuildOnCVSS` set to 7 (High); the unused, severely outdated `bcprov-jdk15on:1.46` dependency (declared in the original `pom.xml`, never imported anywhere in the source) was dropped entirely rather than carried forward |
| Data layer | `DocData` — dead code, hardcoded DB credentials, never actually called | `DocumentRepository` — a real, thread-safe, in-memory `ConcurrentHashMap`. Durable persistence is intentionally out of scope for this artifact; that's the CS-340 database enhancement's job, not this category's |
| Dead code | `customer.java` — unused scaffolding (`showInfo`/`deposit` on a balance that's never initialized, no caller anywhere in the project) | Not carried forward. The code review committed to removing it rather than leaving unused code in the artifact |
| Method naming | `CRUDController.CRUD()` — the method is named identically to its own return type, confusing at any call site | `DocumentController.create()` / `.read()` — named for what they do, not what they return |
| Create path | No create/update/delete endpoint existed despite the class being named `CRUDController` | `POST /api/documents` added. This goes beyond what the Module 1/2 materials explicitly listed, but the role-based-authorization enhancement that *was* explicitly committed to needs a real write operation to actually protect — there was no endpoint for `ADMIN`-only access to mean anything against otherwise |
| Field naming | `CRUD.java` had two fields, `content` and `content2`, which the code review flagged as lazy numbered naming and committed to renaming | **Collapsed to one `content` field instead of renaming both.** The only constructor `CRUDController` ever actually called set both fields to the identical value, so there was never a second distinct piece of data to preserve a name for — renaming `content2` to something descriptive would have been naming a field that held a duplicate of `content` for no reason. Removing the redundant field is a more honest fix than renaming a duplicate; see the narrative for the full reasoning |

## Running it

```
./mvnw spring-boot:run
```

Local dev credentials come from `application.properties` defaults
(`APP_USER_PASSWORD`, `APP_ADMIN_PASSWORD`, `APP_KEYSTORE_PASSWORD` env vars
override them). The committed `keystore.p12` is a fresh self-signed
certificate generated for this artifact — it does not reuse the original
course keystore's password.

## Tests

```
./mvnw test
```

- `DocumentServiceTests` — unit tests for create/read and the not-found path.
- `SecurityIntegrationTests` — an unauthenticated request is rejected; a
  `USER`-role account cannot create documents; an `ADMIN`-role account can,
  and either role can read the result back; a wrong password is rejected.
- `ValidationIntegrationTests` — invalid input is rejected with a 400 and a
  response body that never contains a stack trace or raw exception message.
- `TlsHandshakeTests` — opens a real HTTPS socket (not MockMvc) against the
  running embedded server and confirms the handshake succeeds and the CRUD
  endpoint answers behind it — the actual proof that the merge works, not
  just that the configuration compiles.

## Dependency-check gate

```
./mvnw org.owasp:dependency-check-maven:check
```

Bound to the `verify` phase with `failBuildOnCVSS=7`, so `mvn verify` now
fails the build on any dependency with a known High-or-above severity
vulnerability instead of silently passing regardless of findings.

**This was run for real, not just configured.** A full scan (`./mvnw
org.owasp:dependency-check-maven:10.0.4:check -DfailBuildOnCVSS=7`, against
the live NVD database) found genuine CVSS ≥7.0 findings in several
transitive dependencies pulled in by `spring-boot-starter-parent:3.3.4` -
`jackson-databind`, `log4j-api`, `spring-core`, `spring-security-core`,
`spring-security-web`, `tomcat-embed-core`, and Spring Boot itself - and
**the build failed**, exactly as designed. The full report is
`target/dependency-check-report.html` after running the command above.

This is the gate working, not a defect in this enhancement. The one thing
the code review committed to here was making the plugin capable of failing
the build on a real finding instead of silently passing every time, the
way the original did. It now does that, and this run is the proof. Clearing
every one of these findings would mean a broader dependency-upgrade pass
(potentially a Spring Boot minor-version migration, since several of these
are fixed only in later minor lines) that neither the Module 1 plan nor the
Module 2 code review committed to for this milestone, so it's tracked as a
follow-up rather than added scope tonight: run `mvn verify` before any real
deployment of this service, and address what it finds, not just once but as
an ongoing part of maintaining it.
