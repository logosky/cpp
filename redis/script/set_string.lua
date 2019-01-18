local key_prefix = "demo:string:"

local id = tonumber(ARGV[1])
local value = tonumber(ARGV[2])

local demo_key = key_prefix .. id

local ret = redis.call('SET', demo_key, value)

local return_info = {}
table.insert(return_info, ret)
table.insert(return_info, demo_key)
table.insert(return_info, value)

return return_info
