package com.twk.secureservice.dto;

import java.time.Instant;

/**
 * Every error path returns exactly this shape - status, a stable error code,
 * and a client-safe message. Never an exception's own getMessage() and
 * never a stack trace, which is the gap the enhancement plan calls out
 * explicitly ("error responses never leak internal detail or stack traces").
 */
public class ErrorResponse {

    private final Instant timestamp = Instant.now();
    private final int status;
    private final String error;
    private final String message;

    public ErrorResponse(int status, String error, String message) {
        this.status = status;
        this.error = error;
        this.message = message;
    }

    public Instant getTimestamp() {
        return timestamp;
    }

    public int getStatus() {
        return status;
    }

    public String getError() {
        return error;
    }

    public String getMessage() {
        return message;
    }
}
