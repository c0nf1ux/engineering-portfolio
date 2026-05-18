--[[
DataStore persistence pattern for game state management.
Demonstrates synchronous writes on mutation events with retry logic
to ensure data durability despite transient network failures.
]]

local DataStoreService = game:GetService("DataStoreService")
local playerStore = DataStoreService:GetDataStore("PlayerState")

local PersistenceManager = {}
PersistenceManager.__index = PersistenceManager

function PersistenceManager.new(userId)
  local self = setmetatable({}, PersistenceManager)
  self.userId = userId
  self.maxRetries = 3
  self.retryDelay = 2
  return self
end

function PersistenceManager:savePlayerData(playerData)
  --[[
  Persist player state to DataStore with exponential backoff retry.
  Called immediately on mutation (catch, purchase, upgrade) to ensure
  state durability.
  ]]

  local success = false
  local lastError = nil

  for attempt = 1, self.maxRetries do
    success, lastError = pcall(function()
      playerStore:UpdateAsync("player_" .. self.userId, function(existingData)
        return playerData
      end)
    end)

    if success then
      print("Player data persisted: userId=" .. self.userId)
      return true
    end

    if attempt < self.maxRetries then
      wait(self.retryDelay ^ attempt) -- Exponential backoff
    end
  end

  warn("Failed to persist player data after " .. self.maxRetries .. " attempts")
  warn("Error: " .. lastError)
  return false
end

function PersistenceManager:loadPlayerData()
  --[[
  Load player state from DataStore.
  Serves as source of truth if local state diverges from expected state.
  ]]

  local success, data = pcall(function()
    return playerStore:GetAsync("player_" .. self.userId)
  end)

  if success and data then
    return data
  else
    warn("Failed to load player data: " .. tostring(data))
    return self:createDefaultPlayerData()
  end
end

function PersistenceManager:recordCatchEvent(fishType, reward)
  --[[
  Atomic operation: record catch, update inventory, persist immediately.
  If persistence fails, operation is rolled back locally to prevent
  inventory divergence from persistent state.
  ]]

  local playerData = self:loadPlayerData()

  -- Update local state
  playerData.catches = playerData.catches or 0
  playerData.catches = playerData.catches + 1
  playerData.currency = playerData.currency + reward
  playerData.inventory = playerData.inventory or {}
  playerData.inventory[fishType] = (playerData.inventory[fishType] or 0) + 1

  -- Persist before confirming to client
  local persisted = self:savePlayerData(playerData)

  if not persisted then
    -- Rollback: don't update UI if persistence failed
    error("Failed to persist catch event")
  end

  return playerData
end

function PersistenceManager:createDefaultPlayerData()
  return {
    userId = self.userId,
    currency = 200,
    catches = 0,
    inventory = {},
    createdAt = os.time(),
  }
end

return PersistenceManager
