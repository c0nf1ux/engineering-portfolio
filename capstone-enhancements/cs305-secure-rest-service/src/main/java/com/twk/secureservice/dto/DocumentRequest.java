package com.twk.secureservice.dto;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

/**
 * Real input validation on the create path. The original CRUDController took
 * a raw @RequestParam with no constraints at all and never validated it
 * before use.
 */
public class DocumentRequest {

    @NotBlank(message = "business_name is required")
    @Size(max = 255, message = "business_name must be 255 characters or fewer")
    private String businessName;

    @NotBlank(message = "content is required")
    @Size(max = 10_000, message = "content must be 10,000 characters or fewer")
    private String content;

    public DocumentRequest() {
    }

    public DocumentRequest(String businessName, String content) {
        this.businessName = businessName;
        this.content = content;
    }

    public String getBusinessName() {
        return businessName;
    }

    public void setBusinessName(String businessName) {
        this.businessName = businessName;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }
}
