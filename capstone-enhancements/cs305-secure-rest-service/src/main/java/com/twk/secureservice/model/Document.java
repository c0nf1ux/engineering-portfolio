package com.twk.secureservice.model;

/**
 * Replaces the original DocData, which declared a read_document(key, value)
 * method that opened an unauthenticated JDBC connection
 * ("jdbc:mysql://localhost:3306/test", "root", "root") and then never used
 * the connection, never used its own parameters, and was never actually
 * called by CRUDController - CRUDController.CRUD() called `new DocData()`
 * and returned its default Object#toString(), so the original /read endpoint
 * never read anything.
 *
 * This class is an honest, working replacement scoped to what the software
 * design and engineering enhancement actually calls for: a real, in-memory,
 * thread-safe CRUD path with no hardcoded credentials. Durable persistence
 * (an actual database) is intentionally out of scope here - that is the
 * CS-340 database enhancement artifact's job, not this one's.
 */
public final class Document {

    private final String id;
    private final String businessName;
    private final String content;

    public Document(String id, String businessName, String content) {
        this.id = id;
        this.businessName = businessName;
        this.content = content;
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
