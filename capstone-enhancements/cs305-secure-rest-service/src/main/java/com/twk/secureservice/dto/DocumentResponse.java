package com.twk.secureservice.dto;

import com.twk.secureservice.model.Document;

public class DocumentResponse {

    private final String id;
    private final String businessName;
    private final String content;

    public DocumentResponse(Document document) {
        this.id = document.getId();
        this.businessName = document.getBusinessName();
        this.content = document.getContent();
    }

    public String getId() {
        return id;
    }

    public String getBusinessName() {
        return businessName;
    }

    public String getContent() {
        return content;
    }
}
