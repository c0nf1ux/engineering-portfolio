package com.twk.secureservice.config;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpMethod;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.config.http.SessionCreationPolicy;
import org.springframework.security.core.userdetails.User;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.provisioning.InMemoryUserDetailsManager;
import org.springframework.security.web.SecurityFilterChain;

/**
 * Replaces the course's GameAuth skeleton (a single hardcoded "password" shared
 * by every account) with real per-account credentials, BCrypt hashing, and
 * role-based authorization enforced on the actual CRUD endpoints - none of
 * which the original CRUDController had.
 */
@Configuration
public class SecurityConfig {

    @Value("${app.security.user-password}")
    private String userPassword;

    @Value("${app.security.admin-password}")
    private String adminPassword;

    @Bean
    public PasswordEncoder passwordEncoder() {
        return new BCryptPasswordEncoder();
    }

    @Bean
    public UserDetailsService userDetailsService(PasswordEncoder encoder) {
        var reader = User.withUsername("user")
                .password(encoder.encode(userPassword))
                .roles("USER")
                .build();
        var admin = User.withUsername("admin")
                .password(encoder.encode(adminPassword))
                .roles("USER", "ADMIN")
                .build();
        return new InMemoryUserDetailsManager(reader, admin);
    }

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
            // Stateless JSON API behind TLS, not a browser session - there is no
            // cookie-based session for CSRF to protect.
            .csrf(AbstractHttpConfigurer::disable)
            .sessionManagement(sm -> sm.sessionCreationPolicy(SessionCreationPolicy.STATELESS))
            .authorizeHttpRequests(auth -> auth
                .requestMatchers(HttpMethod.GET, "/api/documents/**").hasAnyRole("USER", "ADMIN")
                .requestMatchers(HttpMethod.POST, "/api/documents/**").hasRole("ADMIN")
                .anyRequest().authenticated())
            .httpBasic(basic -> { });
        return http.build();
    }
}
