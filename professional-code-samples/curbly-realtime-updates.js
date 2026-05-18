/**
 * Real-time location tracking with Socket.io pub/sub pattern.
 * Streams location updates to clients without persisting every update.
 * Only durable state (start, end, summary) written to database.
 */

const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const Database = require('./database');

class RealtimeLocationService {
  constructor(app) {
    this.app = app;
    this.server = http.createServer(app);
    this.io = socketIo(this.server, {
      cors: { origin: '*', credentials: true },
    });
    this.activeTracking = new Map(); // trackingId -> tracking state
    this.database = new Database();

    this.setupSocketHandlers();
  }

  setupSocketHandlers() {
    this.io.on('connection', (socket) => {
      socket.on('start_tracking', async (data) => {
        const { trackingId, userId } = data;
        this.activeTracking.set(trackingId, {
          userId,
          startTime: Date.now(),
          coordinates: [],
        });

        await this.database.createTracking(trackingId, userId);
      });

      socket.on('location_update', (data) => {
        const { trackingId, latitude, longitude, timestamp } = data;

        if (!this.activeTracking.has(trackingId)) {
          return;
        }

        const tracking = this.activeTracking.get(trackingId);
        tracking.coordinates.push({ latitude, longitude, timestamp });

        // Broadcast to all clients watching this tracking
        this.io.to(`tracking_${trackingId}`).emit('location_changed', {
          trackingId,
          latitude,
          longitude,
          timestamp,
        });
      });

      socket.on('end_tracking', async (data) => {
        const { trackingId } = data;
        const tracking = this.activeTracking.get(trackingId);

        if (!tracking) {
          return;
        }

        // Persist only start, end, and route summary to database
        const summary = {
          startTime: tracking.startTime,
          endTime: Date.now(),
          coordinateCount: tracking.coordinates.length,
          startCoordinate: tracking.coordinates[0],
          endCoordinate: tracking.coordinates[tracking.coordinates.length - 1],
        };

        await this.database.completeTracking(trackingId, summary);
        this.activeTracking.delete(trackingId);

        this.io.to(`tracking_${trackingId}`).emit('tracking_completed', summary);
      });

      socket.on('join_tracking', (data) => {
        const { trackingId } = data;
        socket.join(`tracking_${trackingId}`);
      });

      socket.on('leave_tracking', (data) => {
        const { trackingId } = data;
        socket.leave(`tracking_${trackingId}`);
      });
    });
  }

  async startServer(port = 3000) {
    this.server.listen(port, () => {
      console.log(`Real-time tracking service listening on port ${port}`);
    });
  }
}

module.exports = RealtimeLocationService;
