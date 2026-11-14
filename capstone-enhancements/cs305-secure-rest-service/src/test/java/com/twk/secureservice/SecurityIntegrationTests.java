package com.twk.secureservice;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.http.MediaType;
import org.springframework.security.test.web.servlet.request.SecurityMockMvcRequestPostProcessors;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

/**
 * Exercises exactly the hardened paths the enhancement plan calls for:
 * an unauthenticated request is rejected, and role-based authorization is
 * actually enforced on the CRUD endpoints - neither of which the original
 * CRUDController had any way to do, since it had no security configuration
 * at all.
 */
@SpringBootTest
@AutoConfigureMockMvc
class SecurityIntegrationTests {

    @Autowired
    private MockMvc mockMvc;

    @Test
    void unauthenticatedReadIsRejected() throws Exception {
        mockMvc.perform(get("/api/documents").param("business_name", "acme-rescue"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void readerRoleCannotCreateDocuments() throws Exception {
        mockMvc.perform(post("/api/documents")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("user", "changeit-user"))
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"businessName\":\"acme-rescue\",\"content\":\"intake notes\"}"))
                .andExpect(status().isForbidden());
    }

    @Test
    void adminCanCreateAndEitherRoleCanReadItBack() throws Exception {
        mockMvc.perform(post("/api/documents")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("admin", "changeit-admin"))
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"businessName\":\"grazioso-salvare\",\"content\":\"intake notes\"}"))
                .andExpect(status().isCreated());

        mockMvc.perform(get("/api/documents")
                        .param("business_name", "grazioso-salvare")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("user", "changeit-user")))
                .andExpect(status().isOk());
    }

    @Test
    void wrongPasswordIsRejected() throws Exception {
        mockMvc.perform(get("/api/documents")
                        .param("business_name", "acme-rescue")
                        .with(SecurityMockMvcRequestPostProcessors.httpBasic("admin", "not-the-password")))
                .andExpect(status().isUnauthorized());
    }
}
