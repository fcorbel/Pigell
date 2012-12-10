
function generateMap(x, y)
	print("Generating a new map: "..x.."*"..y)
	
	newMap = {}
	for i=1, x do
		newMap[i] = {}
		for j=1, y do
			newMap[i][j] = "water"
		end
	end

	--~ change to earth if not on the border
	for xi,xv in ipairs(newMap) do
		for yi,yv in ipairs(xv) do
			if ((xi > 1) and (yi > 1)) and ((next(xv, yi) ~= nil) and (next(newMap, xi))) then
				newMap[xi][yi] = "earth"
			end
			--~ print("["..xi.."]["..yi.."]", newMap[xi][yi])
		end
	end
	
	return reformat(newMap)
end

function reformat(map)
	mapDef = {}	
	for xi,xv in ipairs(newMap) do
		for yi,yv in ipairs(xv) do
			mapDef[xi..":"..yi] = {}
			mapDef[xi..":"..yi]["x"] = xi-1
			mapDef[xi..":"..yi]["y"] = 0
			mapDef[xi..":"..yi]["z"] = yi-1
			mapDef[xi..":"..yi]["id"] = "matter"
			mapDef[xi..":"..yi]["properties"] = {}
			local matttype
			if map[xi][yi] == "water" then
				matttype = "ocean"
			elseif map[xi][yi] == "earth" then
				matttype = "plain"
			end
			mapDef[xi..":"..yi]["properties"]["matterType"] = matttype
		end
	end	
	
	return mapDef
end

function serialize (o, f, indent)
  if type(o) == "number" then
	f:write(o)
  elseif type(o) == "string" then
	f:write(string.format("%q", o))
  elseif type(o) == "table" then
	f:write("{\n")
	for k,v in pairs(o) do
	  f:write(indent.."  [")
	  serialize(k, f, indent.."  ")
	  f:write("] = ")
	  
	  serialize(v, f, indent.."  ")
	  f:write(",\n")
	end
	f:write(indent.."}")
  else
	error("cannot serialize a " .. type(o))
  end
end


function saveToFile(filename, mapDef)
	file = assert(io.open(filename, "w"))
	file:write("--~A new generated map \n\n")
	file:write("mapDef = ")
	serialize(mapDef, file, "")
end


local genMap = generateMap(50, 50)
saveToFile("newMap.lua", genMap)
