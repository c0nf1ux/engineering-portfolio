package com.twk.secureservice.exception;

public class DocumentNotFoundException extends RuntimeException {

    public DocumentNotFoundException(String businessName) {
        super("No document found for business_name=" + businessName);
    }
}
