struct VSOut
{
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float4 col : COLOR, float2 uv : UV)
{
    VSOut output;

    output.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
    output.color = col;
    output.uv = uv;

    return output;
}