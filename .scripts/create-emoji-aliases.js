import { existsSync } from 'fs'
import { writeFile } from 'fs/promises'
import fetch from 'node-fetch'
import ProxyAgent from 'proxy-agent'
import { fileURLToPath } from 'url'

const __dirname = fileURLToPath(new URL('.', import.meta.url))

const GITHUB_EMOJI_ALIASES_URL = 'https://api.github.com/emojis'

const HTTP_PROXY = process.env.HTTP_PROXY
const httpProxyAgent = HTTP_PROXY && new ProxyAgent(HTTP_PROXY)

/**
 */
const fetchEmojiAliasesFromGithub = async () => {
  /** @type {{ name: string, str: string, charCodes: number[] }[]} */
  const emojis = []

  const emojiAliasesResponse = await fetch(GITHUB_EMOJI_ALIASES_URL, { agent: httpProxyAgent })
  const emojiAliasesJson = /** @type {{ [key: string]: string }} */ (await emojiAliasesResponse.json())

  for (const [alias, url] of Object.entries(emojiAliasesJson)) {
    const match = /unicode\/([\w-]+)\.png/.exec(url)
    if (!match) {
      continue
    }

    const codepoints = match[1].split('-')
    const filename = `${__dirname}/../src/res/72x72/${codepoints.join('-')}.png`
    const emojiStr = String.fromCodePoint(...codepoints.map(c => parseInt(c, 16)))

    const emojiCharCodes = []
    for (let i = 0; i < emojiStr.length; i++) {
      emojiCharCodes.push(emojiStr.charCodeAt(i))
    }

    if (existsSync(filename)) {
      emojis.push({
        name: alias,
        str: emojiStr,
        charCodes: emojiCharCodes,
      })
    }
  }

  return emojis
}

const gitmojiMap = {
  'git:structure': '\\xd83c\\xdfa8',
  'git:format': '\\xd83c\\xdfa8',
  'git:performance': '\\x26a1',
  'git:remove': '\\xd83d\\xdd25',
  'git:delete': '\\xd83d\\xdd25',
  'git:fix': '\\xd83d\\xdc1b',
  'git:bug': '\\xd83d\\xdc1b',
  'git:hotfix': '\\xd83d\\xde91',
  'git:fix:critical': '\\xd83d\\xde91',
  'git:features': '\\x2728',
  'git:documentation': '\\xd83d\\xdcdd',
  'git:add:documentation': '\\xd83d\\xdcdd',
  'git:update:documentation': '\\xd83d\\xdcdd',
  'git:deploy': '\\xd83d\\xde80',
  'git:ui': '\\xd83d\\xdc84',
  'git:add:ui': '\\xd83d\\xdc84',
  'git:update:ui': '\\xd83d\\xdc84',
  'git:styles': '\\xd83d\\xdc84',
  'git:begin': '\\xd83c\\xdf89',
  'git:init': '\\xd83c\\xdf89',
  'git:tests': '\\x2705',
  'git:add:tests': '\\x2705',
  'git:update:tests': '\\x2705',
  'git:security': '\\xd83d\\xdd12',
  'git:fix:security': '\\xd83d\\xdd12',
  'git:version': '\\xd83d\\xdd16',
  'git:release': '\\xd83d\\xdd16',
  'git:warnings': '\\xd83d\\xdea8',
  'git:fix:warnings': '\\xd83d\\xdea8',
  'git:wip': '\\xd83d\\xdea7',
  'git:progress': '\\xd83d\\xdea7',
  'git:fix:ci': '\\xd83d\\xdc9a',
  'git:downgrade_deps': '\\x2b07',
  'git:deps:downgrade': '\\x2b07',
  'git:upgrade_deps': '\\x2b06',
  'git:deps:upgrade': '\\x2b06',
  'git:pin:deps': '\\xd83d\\xdccc',
  'git:deps:pin': '\\xd83d\\xdccc',
  'git:ci': '\\xd83d\\xdc77',
  'git:add:ci': '\\xd83d\\xdc77',
  'git:update:ci': '\\xd83d\\xdc77',
  'git:ci:fix': '\\xd83d\\xdc9a',
  'git:analytics': '\\xd83d\\xdcc8',
  'git:add:analytics': '\\xd83d\\xdcc8',
  'git:update:analytics': '\\xd83d\\xdcc8',
  'git:refactor': '\\x267b',
  'git:add:deps': '\\x2795',
  'git:deps:add': '\\x2795',
  'git:remove:deps': '\\x2796',
  'git:deps:remove': '\\x2796',
  'git:config': '\\xd83d\\xdd27',
  'git:add:config': '\\xd83d\\xdd27',
  'git:update:config': '\\xd83d\\xdd27',
  'git:scripts': '\\xd83d\\xdd28',
  'git:add:scripts': '\\xd83d\\xdd28',
  'git:update:scripts': '\\xd83d\\xdd28',
  'git:internationalization': '\\xd83c\\xdf10',
  'git:localization': '\\xd83c\\xdf10',
  'git:i18n': '\\xd83c\\xdf10',
  'git:typos': '\\x270f',
  'git:fix:typos': '\\x270f',
  'git:poop': '\\xd83d\\xdca9',
  'git:badcode': '\\xd83d\\xdca9',
  'git:workaround': '\\xd83d\\xdca9',
  'git:hack': '\\xd83d\\xdca9',
  'git:revert': '\\x23ea',
  'git:merge': '\\xd83d\\xdd00',
  'git:package': '\\xd83d\\xdce6',
  'git:compiled': '\\xd83d\\xdce6',
  'git:add:compiled': '\\xd83d\\xdce6',
  'git:update:compiled': '\\xd83d\\xdce6',
  'git:external': '\\xd83d\\xdc7d',
  'git:update:external': '\\xd83d\\xdc7d',
  'git:move': '\\xd83d\\xde9a',
  'git:rename': '\\xd83d\\xde9a',
  'git:license': '\\xd83d\\xdcc4',
  'git:add:license': '\\xd83d\\xdcc4',
  'git:update:license': '\\xd83d\\xdcc4',
  'git:breaking': '\\xd83d\\xdca5',
  'git:assets': '\\xd83c\\xdf71',
  'git:add:assets': '\\xd83c\\xdf71',
  'git:update:assets': '\\xd83c\\xdf71',
  'git:accessibility': '\\x267f',
  'git:a11y': '\\x267f',
  'git:comments': '\\xd83d\\xdca1',
  'git:add:comments': '\\xd83d\\xdca1',
  'git:update:comments': '\\xd83d\\xdca1',
  'git:beers': '\\xd83c\\xdf7b',
  'git:drunk': '\\xd83c\\xdf7b',
  'git:text': '\\xd83d\\xdcac',
  'git:add:text': '\\xd83d\\xdcac',
  'git:update:text': '\\xd83d\\xdcac',
  'git:literals': '\\xd83d\\xdcac',
  'git:database': '\\xd83d\\xddc3',
  'git:logs': '\\xd83d\\xdd0a',
  'git:add:logs': '\\xd83d\\xdd0a',
  'git:update:logs': '\\xd83d\\xdd0a',
  'git:logs:remove': '\\xd83d\\xdd07',
  'git:remove:logs': '\\xd83d\\xdd07',
  'git:contributors': '\\xd83d\\xdc65',
  'git:add:contributors': '\\xd83d\\xdc65',
  'git:update:contributors': '\\xd83d\\xdc65',
  'git:ux': '\\xd83d\\xdeb8',
  'git:user_experience': '\\xd83d\\xdeb8',
  'git:usability': '\\xd83d\\xdeb8',
  'git:architecture': '\\xd83c\\xdfd7',
  'git:responsive': '\\xd83d\\xdcf1',
  'git:mock': '\\xd83e\\xdd21',
  'git:easter_egg': '\\xd83e\\xdd5a',
  'gitgitignore': '\\xd83d\\xde48',
  'git:addgitignore': '\\xd83d\\xde48',
  'git:updategitignore': '\\xd83d\\xde48',
  'git:snapshots': '\\xd83d\\xdcf8',
  'git:add:snapshots': '\\xd83d\\xdcf8',
  'git:update:snapshots': '\\xd83d\\xdcf8',
  'git:experiments': '\\x2697',
  'git:seo': '\\xd83d\\xdd0d',
  'git:types': '\\xd83c\\xdff7',
  'git:add:types': '\\xd83c\\xdff7',
  'git:update:types': '\\xd83c\\xdff7',
  'git:seeds': '\\xd83c\\xdf31',
  'git:add:seeds': '\\xd83c\\xdf31',
  'git:update:seeds': '\\xd83c\\xdf31',
  'git:flags': '\\xd83d\\xdea9',
  'git:feature_flags': '\\xd83d\\xdea9',
  'git:add:feature_flags': '\\xd83d\\xdea9',
  'git:update:feature_flags': '\\xd83d\\xdea9',
  'git:remove:feature_flags': '\\xd83d\\xdea9',
  'git:catch_errors': '\\xd83e\\xdd45',
  'git:handle_errors': '\\xd83e\\xdd45',
  'git:animations': '\\xd83d\\xdcab',
  'git:add:animations': '\\xd83d\\xdcab',
  'git:update:animations': '\\xd83d\\xdcab',
  'git:transitions': '\\xd83d\\xdcab',
  'git:deprecate': '\\xd83d\\xddd1',
  'git:authorization': '\\xd83d\\xdec2',
  'git:roles': '\\xd83d\\xdec2',
  'git:permissions': '\\xd83d\\xdec2',
  'git:fix:simple': '\\xd83e\\xde79',
  'git:inspection': '\\xd83e\\xddd0',
  'git:cleanup': '\\x26b0',
  'git:dead_code': '\\x26b0',
}

const GITHUB = process.argv.includes('GITHUB')
const GITMOJI = process.argv.includes('GITMOJI')

if (GITHUB) {
  const githubEmojisIni =
`
[AliasesMap]
${(await fetchEmojiAliasesFromGithub()).map(emoji => `${emoji.name}=${emoji.charCodes.map(c => `\\x${c.toString(16)}`).join('')}`).join('\n')}
`

  await writeFile(`${__dirname}/../src/aliases/github-emojis.ini`, githubEmojisIni.trimStart())
}

if (GITMOJI) {
  const gitmojiEmojisIni =
`
[AliasesList]
${Object.entries(gitmojiMap).map(([key, str], i) => `${i + 1}\\emojiKey=${key}\n${i + 1}\\emojiStr=${str}`).join('\n')}
size=${Object.keys(gitmojiMap).length}
`

  await writeFile(`${__dirname}/../src/aliases/gitmoji-emojis.ini`, gitmojiEmojisIni.trimStart())
}
