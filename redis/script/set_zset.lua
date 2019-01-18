local key_prefix = "demo:zset:"

local member = tostring(ARGV[1])
local score = tonumber(ARGV[2])

local demo_key = key_prefix .. id

local ret = redis.call('ZADD', demo_key, member, score)

local return_info = {}
table.insert(return_info, ret)
table.insert(return_info, demo_key)
table.insert(return_info, member)
table.insert(return_info, score)

return return_info
