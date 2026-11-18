import os
from datetime import datetime

# ════════════════════════════════════════════════════════════════
# CONFIGURAÇÕES
# ════════════════════════════════════════════════════════════════

CONFIG = {
    "pasta_raiz": os.path.dirname(os.path.abspath(__file__)),
    "arquivo_saida": "DOCUMENTACAO_COMPLETA.txt",
    "extensoes_permitidas": [".py", ".html", ".css", ".txt", ".md", ".cfg", ".toml", ".h", ".c"],
    "ignorar": [
        "venv",
        ".venv",
        "__pycache__",
        ".git",
        "staticfiles",
        "db.sqlite3",
        "DOCUMENTACAO_COMPLETA.txt",
        "gerar_documentacao.py",
        ".env",
        "migrations",
    ],
    "tamanho_maximo": 500_000,  # 500 KB
}


def deve_ignorar(caminho):
    nome = os.path.basename(caminho)
    return any(item in caminho or nome == item for item in CONFIG["ignorar"])


def deve_incluir(arquivo):
    _, ext = os.path.splitext(arquivo)
    return ext in CONFIG["extensoes_permitidas"]


def obter_estrutura_pasta(diretorio, nivel=0):
    estrutura = ""
    indent = "  " * nivel

    try:
        itens = sorted(os.listdir(diretorio))
    except Exception as erro:
        return f"Erro ao ler {diretorio}: {erro}\n"

    for item in itens:
        caminho_completo = os.path.join(diretorio, item)

        if deve_ignorar(caminho_completo):
            continue

        if os.path.isdir(caminho_completo):
            estrutura += f"{indent}[PASTA] {item}/\n"
            estrutura += obter_estrutura_pasta(caminho_completo, nivel + 1)
        elif deve_incluir(item):
            tamanho = os.path.getsize(caminho_completo) / 1024
            estrutura += f"{indent}[ARQ] {item} ({tamanho:.2f} KB)\n"

    return estrutura


def obter_todos_arquivos(diretorio, lista_arquivos=None):
    if lista_arquivos is None:
        lista_arquivos = []

    try:
        itens = os.listdir(diretorio)
    except Exception as erro:
        print(f"Erro ao processar {diretorio}: {erro}")
        return lista_arquivos

    for item in itens:
        caminho_completo = os.path.join(diretorio, item)

        if deve_ignorar(caminho_completo):
            continue

        if os.path.isdir(caminho_completo):
            obter_todos_arquivos(caminho_completo, lista_arquivos)
        elif deve_incluir(item):
            tamanho = os.path.getsize(caminho_completo)
            lista_arquivos.append({
                "caminho": caminho_completo,
                "caminho_relativo": os.path.relpath(caminho_completo, CONFIG["pasta_raiz"]),
                "tamanho": tamanho,
                "nome": item,
            })

    return lista_arquivos


def gerar_cabecalho():
    data = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    return f"""
{"=" * 79}
                    DOCUMENTACAO COMPLETA DO PROJETO
{"=" * 79}

Gerado em: {data}
Diretorio: {CONFIG["pasta_raiz"]}

{"=" * 79}

"""


def gerar_indice(arquivos):
    indice = f"""
{"=" * 79}
                          INDICE DE ARQUIVOS
{"=" * 79}

Total de arquivos: {len(arquivos)}

"""
    for i, arquivo in enumerate(arquivos):
        tamanho = arquivo["tamanho"] / 1024
        indice += f"{str(i + 1).zfill(3)}. {arquivo['caminho_relativo']} ({tamanho:.2f} KB)\n"

    indice += f"\n{'=' * 79}\n\n"
    return indice


def gerar_estrutura():
    estrutura = f"""
{"=" * 79}
                      ESTRUTURA DE PASTAS
{"=" * 79}

"""
    estrutura += obter_estrutura_pasta(CONFIG["pasta_raiz"])
    estrutura += f"\n{'=' * 79}\n\n"
    return estrutura


def gerar_conteudo_arquivo(arquivo):
    mod_time = datetime.fromtimestamp(
        os.path.getmtime(arquivo["caminho"])
    ).strftime("%d/%m/%Y %H:%M:%S")

    conteudo = f"""
{"=" * 79}
ARQUIVO: {arquivo["caminho_relativo"]}
{"=" * 79}

Caminho completo: {arquivo["caminho"]}
Tamanho: {arquivo["tamanho"] / 1024:.2f} KB
Ultima modificacao: {mod_time}

{"-" * 79}

"""

    try:
        with open(arquivo["caminho"], "r", encoding="utf-8", errors="replace") as f:
            codigo = f.read()

        if arquivo["tamanho"] > CONFIG["tamanho_maximo"]:
            linhas = codigo.splitlines()
            codigo = "\n".join(linhas[:1000])
            codigo += "\n\n[ARQUIVO TRUNCADO: mostrando apenas as primeiras 1000 linhas]\n"

        conteudo += codigo

    except Exception as erro:
        conteudo += f"ERRO AO LER ARQUIVO: {erro}\n"

    conteudo += f"\n\n{'=' * 79}\n"
    return conteudo


def gerar_estatisticas(arquivos):
    por_extensao = {}
    tamanho_total = 0

    for arquivo in arquivos:
        _, extensao = os.path.splitext(arquivo["nome"])
        por_extensao[extensao] = por_extensao.get(extensao, 0) + 1
        tamanho_total += arquivo["tamanho"]

    estatisticas = f"""
{"=" * 79}
                          ESTATISTICAS
{"=" * 79}

Total de arquivos: {len(arquivos)}
Tamanho total: {tamanho_total / 1024 / 1024:.2f} MB

Arquivos por tipo:
"""

    for extensao, quantidade in sorted(
        por_extensao.items(),
        key=lambda item: item[1],
        reverse=True,
    ):
        estatisticas += f"   {extensao or '[sem extensao]'}: {quantidade} arquivo(s)\n"

    estatisticas += f"\n{'=' * 79}\n\n"
    return estatisticas


def executar():
    print("Iniciando a geracao da documentacao...\n")
    inicio = datetime.now()

    print("Escaneando arquivos...")
    arquivos = obter_todos_arquivos(CONFIG["pasta_raiz"])
    arquivos.sort(key=lambda arquivo: arquivo["caminho_relativo"].lower())
    print(f"{len(arquivos)} arquivos encontrados.\n")

    documentacao = ""
    documentacao += gerar_cabecalho()
    documentacao += gerar_estatisticas(arquivos)
    documentacao += gerar_estrutura()
    documentacao += gerar_indice(arquivos)

    print("Adicionando conteudo dos arquivos...")
    for numero, arquivo in enumerate(arquivos, start=1):
        print(f"[{numero}/{len(arquivos)}] {arquivo['caminho_relativo']}")
        documentacao += gerar_conteudo_arquivo(arquivo)

    caminho_saida = os.path.join(
        CONFIG["pasta_raiz"],
        CONFIG["arquivo_saida"],
    )

    with open(caminho_saida, "w", encoding="utf-8") as f:
        f.write(documentacao)

    duracao = (datetime.now() - inicio).total_seconds()
    tamanho_saida = os.path.getsize(caminho_saida)

    print("\nConcluido!")
    print(f"Arquivo gerado: {caminho_saida}")
    print(f"Tamanho: {tamanho_saida / 1024 / 1024:.2f} MB")
    print(f"Tempo: {duracao:.2f} segundos")


if __name__ == "__main__":
    try:
        executar()
    except Exception as erro:
        print(f"Erro fatal: {erro}")
        raise