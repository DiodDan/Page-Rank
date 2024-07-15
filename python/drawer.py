from faker import Faker
from pathlib import Path
from tqdm import tqdm

faker = Faker()

pages = 300
edge_list = []

splitter = ";"

nodes_data = ""
edges_data = ""

connection_type = "Directed"


# def random_page():
#     for i in range(pages):
#         nodes_data += f"{i}{splitter}{faker.word()}\n"
#         for _ in range(20):
#             connect_page = faker.random_int(0, pages)
#             weight = faker.random_int(1, 10)
#             if connect_page != i:
#                 edges_data += f"{i}{splitter}{connect_page}{splitter}{connection_type}{splitter}{weight}\n"

edges = set()
for i in tqdm((Path(__file__).parent.parent / "data" / "links.txt").read_text().split("\n")):
    source, target = i.split(" -> ")
    source = source.replace(".html", "")
    target = target.replace(".html", "")
    if source not in edges:
        edges.add(source)
        nodes_data += f"{source}{splitter}{source}\n"
    if target not in edges:
        edges.add(target)
        nodes_data += f"{target}{splitter}{target}\n"

    if source != target:
        edges_data += f"{source}{splitter}{target}{splitter}{connection_type}{splitter}1\n"

nodes_path = Path(__file__).parent.parent / "data" / "nodes.csv"
edges_path = Path(__file__).parent.parent / "data" / "edges.csv"


nodes_path.write_text(f'Id{splitter}Label\n' + nodes_data)
edges_path.write_text(f'Source{splitter}Target{splitter}Type{splitter}Weight\n' + edges_data)
