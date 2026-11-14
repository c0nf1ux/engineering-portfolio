package com.twk.secureservice;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.http.MediaType;
import org.springframework.security.test.web.servlet.request.SecurityMockMvcRequestPostProcessors;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

/**
 * Confirms invalid input is rejected cleanly with a stable, client-safe
 * error body - and, just as importantly, that the body never contains a
 * stack trace or an exception's raw message. The original /read endpoint
 * took an unvalidated @RequestParam and would have let a request-handling
 * exception propagate straight to Spring Boot's default whitelabel error
 * page, which includes the exception class and message.
 */
@SpringBootTest
@AutoConfigureMockMvc
class ValidationIntegrationTests {

    @Autowired
    private MockMvc mockMvc;

    @Test
    void blankBusinessNameIsRejectedWithNoStackTrace() throws Exception {
        mockMvc.perform(post("/api/documents")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("admin", "changeit-admin"))
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"businessName\":\"\",\"content\":\"intake notes\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.status").value(400))
                .andExpect(jsonPath("$.message").exists())
                .andExpect(jsonPath("$.stackTrace").doesNotExist())
                .andExpect(jsonPath("$.trace").doesNotExist());
    }

    @Test
    void oversizedContentIsRejected() throws Exception {
        String tooLong = "a".repeat(10_001);
        mockMvc.perform(post("/api/documents")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("admin", "changeit-admin"))
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"businessName\":\"acme-rescue\",\"content\":\"" + tooLong + "\"}"))
                .andExpect(status().isBadRequest());
    }
}
