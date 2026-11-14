package com.twk.secureservice;

import com.twk.secureservice.dto.DocumentRequest;
import com.twk.secureservice.exception.DocumentNotFoundException;
import com.twk.secureservice.repository.DocumentRepository;
import com.twk.secureservice.service.DocumentService;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

class DocumentServiceTests {

    private final DocumentService service = new DocumentService(new DocumentRepository());

    @Test
    void createThenReadReturnsTheSameDocument() {
        service.create(new DocumentRequest("acme-rescue", "intake notes"));

        var response = service.read("acme-rescue");

        assertEquals("acme-rescue", response.getBusinessName());
        assertEquals("intake notes", response.getContent());
    }

    @Test
    void readingAnUnknownBusinessNameThrows() {
        assertThrows(DocumentNotFoundException.class, () -> service.read("does-not-exist"));
    }
}
