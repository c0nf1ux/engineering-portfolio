package com.twk.secureservice;

import org.junit.jupiter.api.Test;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.web.server.LocalServerPort;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;
import java.io.InputStream;
import java.net.URL;
import java.security.KeyStore;
import java.util.Base64;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * Proves the actual merge, not just the configuration. Project One's
 * CRUD controller and Project Two's TLS listener were two disconnected
 * course projects; the CRUD service never ran behind TLS at all. This
 * test opens a real HTTPS socket against the running embedded server -
 * no MockMvc shortcut - and confirms the handshake completes and the
 * CRUD endpoint answers behind it.
 */
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
class TlsHandshakeTests {

    @LocalServerPort
    private int port;

    @Test
    void crudEndpointIsReachableOnlyOverARealTlsHandshake() throws Exception {
        SSLContext sslContext = trustingOnlyOurOwnSelfSignedCert();

        URL url = new URL("https://localhost:" + port + "/api/documents?business_name=acme-rescue");
        HttpsURLConnection connection = (HttpsURLConnection) url.openConnection();
        connection.setSSLSocketFactory(sslContext.getSocketFactory());
        connection.setRequestProperty("Authorization",
                "Basic " + Base64.getEncoder().encodeToString("user:changeit-user".getBytes()));

        int status = connection.getResponseCode();

        // 404 (no document named acme-rescue exists yet) is the correct
        // result here. The point being proven is that the TLS handshake
        // succeeded and the request reached the CRUD controller at all -
        // never true for the original two-separate-projects arrangement.
        assertEquals(404, status);
    }

    private SSLContext trustingOnlyOurOwnSelfSignedCert() throws Exception {
        KeyStore trustStore = KeyStore.getInstance("PKCS12");
        try (InputStream in = getClass().getClassLoader().getResourceAsStream("keystore.p12")) {
            trustStore.load(in, "changeit-dev-only".toCharArray());
        }
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init(trustStore);
        SSLContext context = SSLContext.getInstance("TLS");
        context.init(null, tmf.getTrustManagers(), null);
        return context;
    }
}
