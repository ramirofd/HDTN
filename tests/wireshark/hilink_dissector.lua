-- HiLink convergence layer dissector for Wireshark 4.4.x
-- Parses HDTN HiLink UDP frames that prepend a single header byte to a BP bundle.

local hilink_proto = Proto("hilink", "HDTN HiLink")

local f_header = ProtoField.uint8("hilink.header", "Header Byte", base.HEX)
local f_payload = ProtoField.bytes("hilink.payload", "Bundle Payload")

hilink_proto.fields = { f_header, f_payload }

local default_udp_port = 4556
hilink_proto.prefs.udp_port = Pref.uint("UDP port", default_udp_port, "UDP port used for HiLink bundles (0 disables the automatic binding).", 0, 65535)
hilink_proto.prefs.expected_header = Pref.uint("Expected header", 0xA0, "Expected value of the HiLink header byte used for highlighting mismatches.", 0x00, 0xFF)

local udp_table = DissectorTable.get("udp.port")
local current_port = default_udp_port

function hilink_proto.init()
    if current_port and current_port > 0 then
        pcall(function() udp_table:remove(current_port, hilink_proto) end)
    end
    current_port = hilink_proto.prefs.udp_port
    if current_port and current_port > 0 then
        udp_table:add(current_port, hilink_proto)
    end
end

function hilink_proto.prefs_changed()
    hilink_proto.init()
end

function hilink_proto.dissector(buffer, pinfo, tree)
    if buffer:len() < 1 then
        return
    end

    pinfo.cols.protocol = "HILINK"

    local subtree = tree:add(hilink_proto, buffer(), "HiLink Bundle")
    local header_range = buffer(0, 1)
    local header_item = subtree:add(f_header, header_range)
    local header_value = header_range:uint()
    local expected = hilink_proto.prefs.expected_header
    if expected and header_value ~= expected then
        header_item:append_text(string.format(" (expected 0x%02X)", expected))
    end

    if buffer:len() > 1 then
        subtree:add(f_payload, buffer(1)):set_text("Bundle Payload (" .. (buffer:len() - 1) .. " bytes)")
    else
        subtree:add(f_payload, buffer(1, 0))
    end
end

hilink_proto.init()

return hilink_proto
