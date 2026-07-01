#!/bin/bash

echo "=== ANÁLISE ESTÁTICA ==="

# 1. delay() proibido
if grep -rn "delay(" src/ --include="*.cpp" --include="*.h" | grep -v "//"; then
  echo "❌ FALHA: delay() encontrado"
else
  echo "✅ sem delay()"
fi

# 2. String (tipo) proibido
if grep -rn "\bString\b" src/ --include="*.cpp" --include="*.h" | grep -v "//"; then
  echo "❌ FALHA: tipo String encontrado"
else
  echo "✅ sem String"
fi

# 3. lcd.print sem F() para literais
if grep -rn 'lcd\.print("' src/ --include="*.cpp"; then
  echo "❌ FALHA: lcd.print() com literal sem F()"
else
  echo "✅ F() usado corretamente"
fi

# 4. Include guards em todos os .h
for f in src/**/*.h include/*.h; do
  if ! grep -q "#ifndef" "$f" 2>/dev/null; then
    echo "❌ FALHA: $f sem include guard"
  fi
done
echo "✅ include guards verificados"

# 5. Memória (só o ambiente uno gera firmware real; native é só para testes)
echo ""
echo "=== USO DE MEMÓRIA ==="
pio run -e uno 2>&1 | grep -E "RAM:|Flash:"
