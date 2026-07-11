#!/bin/bash
echo "=== Testing Chennai Lang API ==="
curl -s -X POST http://localhost/chennai_lang_onAWS/api/run \
  -H 'Content-Type: application/json' \
  -d '{"code":"polamanna\nmain() {\n    \"Hello from Chennai Lang v2!\" sollu\n}\nniruthuanna"}' | python3 -m json.tool

echo ""
echo "=== Testing examples endpoint ==="
curl -s http://localhost/chennai_lang_onAWS/api/examples | python3 -c "import sys,json; d=json.load(sys.stdin); print('Examples found:', list(d['examples'].keys()))"

echo ""
echo "=== Container logs (last 20) ==="
sudo docker logs chennai-lang --tail 20
