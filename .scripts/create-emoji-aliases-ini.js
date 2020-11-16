const { promises: { writeFile }, existsSync } = require('fs')
const { default: fetch } = require('node-fetch')
const ProxyAgent = require('proxy-agent')

const GITHUB_EMOJI_ALIASES_URL = 'https://api.github.com/emojis'

const HTTP_PROXY = process.env.HTTP_PROXY
const httpProxyAgent = HTTP_PROXY && new ProxyAgent(HTTP_PROXY)

/**
 * @returns {Promise<{ name: string, str: string, charCodes: number[] }[]>}
 */
const fetchEmojiAliasesFromGithub = async () => {
  const emojis = []

  const emojiAliasesResponse = await fetch(GITHUB_EMOJI_ALIASES_URL, { agent: httpProxyAgent })
  const emojiAliasesRegex = /^\s*"([^"]+)": "https:\/\/github\.githubassets\.com\/images\/icons\/emoji\/unicode\/([^.]+)\.png\?v8"/gm
  const emojiAliasesTxt = await emojiAliasesResponse.text()

  let match
  while ((match = emojiAliasesRegex.exec(emojiAliasesTxt)) !== null) {
    if (match.index === emojiAliasesRegex.lastIndex) {
      emojiAliasesRegex.lastIndex++
    }

    const codepoints = match[2].split('-')
    const filename = `${__dirname}/../src/res/72x72/${codepoints.join('-')}.png`
    const emojiStr = String.fromCodePoint(...codepoints.map(c => parseInt(c, 16)))

    const emojiCharCodes = []
    for (let i = 0; i < emojiStr.length; i++) {
      emojiCharCodes.push(emojiStr.charCodeAt(i))
    }

    if (existsSync(filename)) {
      emojis.push({
        name: `:${match[1]}:`,
        str: emojiStr,
        charCodes: emojiCharCodes,
      })
    }
  }

  return emojis
}

(async () => {
  const GITHUB = process.argv.includes('GITHUB')

  if (GITHUB) {
    const githubEmojisIni =
`
[Aliases]
${(await fetchEmojiAliasesFromGithub()).map(emoji => `${emoji.name}=${emoji.charCodes.map(c => `\\x${c.toString(16)}`).join('')}`).join('\n')}
`

    await writeFile(`${__dirname}/../src/aliases/github-emojis.ini`, githubEmojisIni.trimLeft())
  }
})().catch(console.error)
