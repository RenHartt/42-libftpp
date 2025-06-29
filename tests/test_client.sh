#!/usr/bin/env bash
set -e

# --- Génération automatique du serveur de test Python ---
cat > server_test.py << 'EOF'
#!/usr/bin/env python3
import socket, struct, sys

HOST = '127.0.0.1'
PORT = 8080

def recvall(sock, n):
    data = b''
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data += packet
    return data

# Helper pour extraire un segment préfixé par un size_t (8 bytes little-endian)
def extract_segment(payload, offset):
    seg_len = struct.unpack_from('<Q', payload, offset)[0]
    start = offset + 8
    end = start + seg_len
    return payload[start:end], end

print(f"[Server] Démarrage sur {HOST}:{PORT}", file=sys.stderr, flush=True)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()
    print(f"[Server] Client connecté : {addr}", file=sys.stderr, flush=True)
    with conn:
        while True:
            header = recvall(conn, 8)
            if not header:
                print("[Server] Fin de connexion", file=sys.stderr, flush=True)
                break

            # Lecture du header en big-endian (network order)
            msg_type, length = struct.unpack('!II', header)
            payload = recvall(conn, length)
            print(f"[Server] Reçu  type={msg_type}, length={length}", file=sys.stderr, flush=True)

            if msg_type == 1:
                # Type 1 : un int (segment unique)
                seg, _ = extract_segment(payload, 0)
                val = int(seg.decode('ascii'))
                print(f"[Server]  → int = {val}", file=sys.stderr, flush=True)
                doubled = val * 2

                # Construction réponse type=3 (int)
                resp_str = str(doubled).encode('ascii')
                resp_payload = struct.pack('<Q', len(resp_str)) + resp_str
                resp_header  = struct.pack('!II', 3, len(resp_payload))
                conn.sendall(resp_header + resp_payload)
                print(f"[Server] Envoyé type=3 doubled={doubled}", file=sys.stderr, flush=True)

            elif msg_type == 2:
                # Type 2 : une string avec deux segments (taille + texte)
                seg1, off = extract_segment(payload, 0)
                str_len = int(seg1.decode('ascii'))
                print(f"[Server]  → attendu string length = {str_len}", file=sys.stderr, flush=True)

                # Extraction du segment de la chaîne complète
                seg2, _ = extract_segment(payload, off)
                s_data = seg2.decode('ascii')
                print(f"[Server]  → string = '{s_data}'", file=sys.stderr, flush=True)

                # Echo en type=3
                echo_bytes = s_data.encode('ascii')
                resp_payload = struct.pack('<Q', len(echo_bytes)) + echo_bytes
                resp_header  = struct.pack('!II', 3, len(resp_payload))
                conn.sendall(resp_header + resp_payload)
                print(f"[Server] Envoyé type=3 echo='{s_data}'", file=sys.stderr, flush=True)

            else:
                print(f"[Server]  → type non géré ({msg_type})", file=sys.stderr, flush=True)
EOF
chmod +x server_test.py

# --- Lancement du serveur (unbuffered + tee pour avoir les logs en direct) ---
echo "→ Démarrage du serveur de test…"
stdbuf -oL python3 server_test.py 2>&1 | tee server.log &
SERVER_PID=$!
sleep 1

# --- Exécution du client ---
echo "→ Lancement du client…"
./bin/main_client

# --- Nettoyage et affichage des logs ---
echo
echo "→ Arrêt du serveur (PID $SERVER_PID)"
kill $SERVER_PID 2>/dev/null || true
echo
echo "=== Logs complets du serveur ==="
cat server.log
echo "=== Fin des logs ==="