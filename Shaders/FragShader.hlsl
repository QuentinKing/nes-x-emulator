Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

float3 hash(float2 uv)
{
    float2 smallValue = sin(uv);
    float x = frac(sin(dot(smallValue, float2(13.74, 91.23))) * 143798.123);
    float y = frac(sin(dot(smallValue, float2(23.35, 81.79))) * 143798.123);
    float z = frac(sin(dot(smallValue, float2(33.57, 71.11))) * 143798.123);
    return float3(x,y,z);
}

float4 main(float4 color : COLOR, float2 uv : TEXCOORD0) : SV_TARGET
{
    float yOffset = (fmod(round(uv.x * 256.0), 2.0)) * 0.0002;
    float xOffset = 0.002;

    uv -= 0.5; // Center uv coordinates since it makes the math easier

    float intensity = length(uv);
    intensity = clamp(pow(intensity, 8.0), 0.0, 1.0);
    uv = lerp(uv, uv + normalize(uv), intensity);
    uv *= 0.92;

    // Grain
    float2 pixelCoord = float2(round(uv.x * 256.0) / 256.0, round(uv.y * 240.0) / 240.0);

    float r = txDiffuse.Sample(samLinear, uv + 0.5 - float2(xOffset, yOffset)).x - (pow(length(uv), 1.0) * 0.5);
    float g = txDiffuse.Sample(samLinear, uv + 0.5 - float2(0.00, yOffset)).y - (pow(length(uv), 1.0) * 0.5);
    float b = txDiffuse.Sample(samLinear, uv + 0.5 - float2(-xOffset, yOffset)).z - (pow(length(uv), 1.0) * 0.5);

    return float4(r,g,b,1.0) + (float4(hash(pixelCoord) * 0.02, 0.0));
    //return color;
}