package com.twk.secureservice.service;

import com.twk.secureservice.dto.DocumentRequest;
import com.twk.secureservice.dto.DocumentResponse;
import com.twk.secureservice.exception.DocumentNotFoundException;
import com.twk.secureservice.model.Document;
import com.twk.secureservice.repository.DocumentRepository;
import org.springframework.stereotype.Service;

import java.util.UUID;

@Service
public class DocumentService {

    private final DocumentRepository repository;

    public DocumentService(DocumentRepository repository) {
        this.repository = repository;
    }

    public DocumentResponse create(DocumentRequest request) {
        Document document = new Document(
                UUID.randomUUID().toString(),
                request.getBusinessName(),
                request.getContent());
        return new DocumentResponse(repository.save(document));
    }

    public DocumentResponse read(String businessName) {
        return repository.findByBusinessName(businessName)
                .map(DocumentResponse::new)
                .orElseThrow(() -> new DocumentNotFoundException(businessName));
    }
}
