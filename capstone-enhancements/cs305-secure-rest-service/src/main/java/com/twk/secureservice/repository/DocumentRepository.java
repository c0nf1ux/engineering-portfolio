package com.twk.secureservice.repository;

import com.twk.secureservice.model.Document;
import org.springframework.stereotype.Repository;

import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;

@Repository
public class DocumentRepository {

    private final Map<String, Document> documentsByBusinessName = new ConcurrentHashMap<>();

    public Document save(Document document) {
        documentsByBusinessName.put(document.getBusinessName(), document);
        return document;
    }

    public Optional<Document> findByBusinessName(String businessName) {
        return Optional.ofNullable(documentsByBusinessName.get(businessName));
    }

    public boolean existsByBusinessName(String businessName) {
        return documentsByBusinessName.containsKey(businessName);
    }
}
