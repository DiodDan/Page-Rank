import os
from bs4 import BeautifulSoup
from urllib.parse import urlparse, urljoin
from pathlib import Path
from tqdm import tqdm


def get_html_files(directory):
    return list({f for f in os.listdir(directory) if f.endswith('.html')})


def extract_links(html_content, base_url):
    soup = BeautifulSoup(html_content, 'html.parser')
    links = []

    for a_tag in soup.find_all('a', href=True):
        href = a_tag['href']
        parsed_href = urlparse(href)

        if parsed_href.netloc:
            continue

        full_url = urljoin(base_url, href)
        links.append(full_url)

    return links


def create_links_list(directory):
    html_files = get_html_files(directory)
    links_list = []

    for html_file in tqdm(html_files):
        file_path = os.path.join(directory, html_file)
        with open(file_path, 'r', encoding='utf-8') as file:
            html_content = file.read()

        links = extract_links(html_content, os.path.abspath(file_path))
        for link in links:
            file = link[6:] + ".html"
            if file in html_files:
                links_list.append((html_file, file))

    return links_list


def main():
    directory = Path(__file__).parent.parent / 'wikipedia_pages'
    links_list = create_links_list(directory)

    information = "\n".join([f'{source} -> {target}' for source, target in links_list])
    print(information)

    Path('links.txt').write_text(information)



if __name__ == '__main__':
    main()
