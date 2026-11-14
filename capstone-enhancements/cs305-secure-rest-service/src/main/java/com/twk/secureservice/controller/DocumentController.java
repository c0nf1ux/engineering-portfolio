package com.twk.secureservice.controller;

import com.twk.secureservice.dto.DocumentRequest;
import com.twk.secureservice.dto.DocumentResponse;
import com.twk.secureservice.service.DocumentService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * Replaces CRUDController. The original exposed a single ad hoc GET /read
 * endpoint with no request body support, no validation, and no
 * authentication; this models the same resource as a real REST collection
 * (/api/documents) with create (admin-only) and read (any authenticated
 * reader), both enforced by SecurityConfig, both running over the TLS
 * listener merged in from Project Two.
 */
@RestController
@RequestMapping("/api/documents")
@Validated
public class DocumentController {

    private final DocumentService documentService;

    public DocumentController(DocumentService documentService) {
        this.documentService = documentService;
    }

    @PostMapping
    public ResponseEntity<DocumentResponse> create(@Valid @RequestBody DocumentRequest request) {
        return ResponseEntity.status(HttpStatus.CREATED).body(documentService.create(request));
    }

    @GetMapping
    public ResponseEntity<DocumentResponse> read(
            @RequestParam("business_name") @NotBlank String businessName) {
        return ResponseEntity.ok(documentService.read(businessName));
    }
}
