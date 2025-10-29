-- HiLink TCP convergence layer dissector for Wireshark 4.4.x
-- Parses HDTN HiLink TCP frames that wrap bundles with a header byte and a 4-byte length prefix.

local hilink_tcp_proto = Proto("hilink_tcp", "HDTN HiLink TCP")

local f_header = ProtoField.uint8("hilink_tcp.header", "Header Byte", base.HEX)
local f_length = ProtoField.uint32("hilink_tcp.length", "Bundle Length", base.DEC)
local f_payload = ProtoField.bytes("hilink_tcp.payload", "Bundle Payload")

hilink_tcp_proto.fields = { f_header, f_length, f_payload }

local default_tcp_port = 4556
hilink_tcp_proto.prefs.tcp_port = Pref.uint("TCP port", default_tcp_port, "TCP port used for HiLink TCP bundles (0 disables the automatic binding).", 0, 65535)
hilink_tcp_proto.prefs.expected_header = Pref.uint("Expected header", 0xA0, "Expected value of the HiLink header byte used for highlighting mismatches.", 0x00, 0xFF)

local tcp_table = DissectorTable.get("tcp.port")
local current_port = default_tcp_port

function hilink_tcp_proto.init()
    if current_port and current_port > 0 then
        pcall(function() tcp_table:remove(current_port, hilink_tcp_proto) end)
    end
    current_port = hilink_tcp_proto.prefs.tcp_port
    if current_port and current_port > 0 then
        tcp_table:add(current_port, hilink_tcp_proto)
    end
end

function hilink_tcp_proto.prefs_changed()
    hilink_tcp_proto.init()
end

function hilink_tcp_proto.dissector(buffer, pinfo, tree)
    local total_len = buffer:len()
    if total_len == 0 then
        return
    end

    pinfo.cols.protocol = "HILINK-TCP"

    local offset = 0
    while offset < total_len do
        local remaining = total_len - offset
        if remaining < 5 then
            pinfo.desegment_offset = offset
            pinfo.desegment_len = 5 - remaining
            return
        end

        local header_range = buffer(offset, 1)
        local declared_length = buffer(offset + 1, 4):uint()
        local frame_length = 5 + declared_length

        if remaining < frame_length then
            pinfo.desegment_offset = offset
            pinfo.desegment_len = frame_length - remaining
            return
        end

        local subtree = tree:add(hilink_tcp_proto, buffer(offset, frame_length), string.format("HiLink TCP Bundle (%d bytes payload)", declared_length))
        local header_item = subtree:add(f_header, header_range)
        local expected = hilink_tcp_proto.prefs.expected_header
        local header_value = header_range:uint()
        if expected and header_value ~= expected then
            header_item:append_text(string.format(" (expected 0x%02X)", expected))
        end
        subtree:add(f_length, buffer(offset + 1, 4))
        if declared_length > 0 then
            subtree:add(f_payload, buffer(offset + 5, declared_length))
        else
            subtree:add(f_payload, buffer(offset + 5, 0))
        end

        offset = offset + frame_length
    end
end

hilink_tcp_proto.init()

return hilink_tcp_proto
